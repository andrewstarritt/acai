/* buffered_callbacks.c
 *
 * EPICS buffered callback module for use with Ada, Lazarus and other
 * runtime environments which don't like alien threads.
 *
 * SPDX-FileCopyrightText: 2005-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 */

/* ---------------------------------------------------------------------------
 * Description:
 * The module buffers the channel access callbacks.
 * The registered callback handlers store a copy of the call back data.
 * The actual call backs are initiated via process_buffered_callbacks
 * from a native application thread.
 *
 * This ensures that the actual call back runs within an application
 * thread as opposed to a libca.so shared library thread.
 *
 * Rational: The main application runtime environment does not allow
 * code to do anything significant if running within an alien thread.
 *
 * Source code formatting:
 *    indent -kr -pcs -i3 -cli3 -nbbo -nut
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cadef.h>
#include <caerr.h>
#include <ellLib.h>
#include <epicsMutex.h>

#include "buffered_callbacks.h"

/* These functions must be exported by the main application.
 *
 * Note: unlike the equivelent raw channel access callback functions,
 *       these functions always take POINTER arguments.
 */
extern void application_connection_handler (struct connection_handler_args* ptr);

extern void application_event_handler (struct event_handler_args* ptr);

extern void application_printf_handler (const char* formatted_text);


/* -----------------------------------------------------------------------------
 * PRIVATE - implementation details
 * -----------------------------------------------------------------------------
 */

/* Buffered stuff kinds
 */
typedef enum Callback_Kinds {
   NULL_KIND,
   CONNECTION,
   EVENT,
   PRINTF
} Callback_Kinds;


typedef struct Callback_Items {
   ELLNODE ellnode;
   Callback_Kinds kind;

   /* perhaps we could use a union here
    */
   struct connection_handler_args cargs;
   struct event_handler_args eargs;
   char *formatted_text;
} Callback_Items;


/*------------------------------------------------------------------------------
 * Module data
 */
static epicsMutexId linked_list_mutex = NULL;
static ELLLIST linked_list = ELLLIST_INIT;
static unsigned int allocate_fail_count = 0;
static unsigned int multiple_check_limit = 1000;
static unsigned int discard_count = 0;


/*------------------------------------------------------------------------------
 * Allocate and initialise call back item
 */
static Callback_Items *allocate_element (const Callback_Kinds kind)
{
   Callback_Items *pci;

   pci = (Callback_Items *) malloc (sizeof (Callback_Items));
   if (pci) {
      pci->kind = kind;
      /* Just do all pointers irrespective of kind
       */
      pci->eargs.dbr = NULL;
      pci->formatted_text = NULL;
   } else {
      /* Technically we should protect this with a mutex, but only used
       * as diagnostic so do not have to be that strict.
       */
      allocate_fail_count++;
   }

   return pci;
}                               /* allocate_element */


/*------------------------------------------------------------------------------
 * free_element - frees all dynamic data associated with this element.
 */
static void free_element (Callback_Items * pci)
{
   switch (pci->kind) {

      case CONNECTION:
         /* No special action */
         break;

      case EVENT:
         if (pci->eargs.dbr) {
            free ((void *) pci->eargs.dbr);
            pci->eargs.dbr = NULL;
         }
         break;

      case PRINTF:
         if (pci->formatted_text) {
            free (pci->formatted_text);
            pci->formatted_text = NULL;
         }
         break;

      default:
         /* What the ....? */
         break;

   }

   free (pci);
}                               /* free_element */


/*------------------------------------------------------------------------------
 */
static void load_element (Callback_Items* pci)
{
   /* Gain exclusive access to linked list
    */
   epicsMutexLock (linked_list_mutex);

   /* Search list for existing update for same channel and remove if found.
    */
   if ((pci->kind == EVENT) && (ellCount (&linked_list) > multiple_check_limit)) {
      /* Search item must match kind, chid, usr, and type
       */
      struct ELLNODE* check = ellFirst (&linked_list);
      while (check) {
         Callback_Items* ci = (Callback_Items *) check;

         if ((ci->kind == EVENT) &&
             (ci->eargs.chid == pci->eargs.chid) &&
             (ci->eargs.type == pci->eargs.type) &&
             (ci->eargs.usr == pci->eargs.usr))
         {
            /* we have a match - remove the earliest previous update
             */
            ellDelete (&linked_list, check);
            free_element (ci);
            discard_count++;

            break;  /* we need only search for one. */
         }
         check = ellNext (check);
      }
   }

   ellAdd (&linked_list, (ELLNODE *) pci);

   /* Release exclusive access to linked list
    */
   epicsMutexUnlock (linked_list_mutex);
}                               /* load_element */


/*------------------------------------------------------------------------------
 * unload - is NULL if nothing in the list.
 */
static Callback_Items *unload_element ()
{
   Callback_Items *result;

   /* Gain exclusive access to linked list
    */
   epicsMutexLock (linked_list_mutex);

   result = (Callback_Items *) ellGet (&linked_list);

   /* Release exclusive access to linked list
    */
   epicsMutexUnlock (linked_list_mutex);

   return result;
}                               /* unload_element */


/*------------------------------------------------------------------------------
 * Connection handler
 */
void buffered_connection_handler (struct connection_handler_args args)
{
   Callback_Items *pci;

   pci = allocate_element (CONNECTION);
   if (pci) {

      /* Copy all fields. */
      pci->cargs = args;

      load_element (pci);
   }
}                               /* buffered_connection_handler */


/*------------------------------------------------------------------------------
 * Event handler
 */
void buffered_event_handler (struct event_handler_args args)
{
   Callback_Items *pci;
   size_t size;

   pci = allocate_element (EVENT);
   if (pci) {

      /* Copy all fields. */
      pci->eargs = args;

      /* Calculate size of dbr field, and alloc memory for copy iff required
       */
      if (args.dbr != NULL) {
         size = dbr_size_n (args.type, args.count);
         pci->eargs.dbr = malloc (size);
         memcpy ((void *) pci->eargs.dbr, args.dbr, size);
      }

      load_element (pci);
   }
}                               /* buffered_event_handler */


/*------------------------------------------------------------------------------
 * Replacement printf handler
 */
int buffered_printf_handler (const char *pformat, va_list args)
{
   Callback_Items *pci;
   /* Expanded strings never more than 80, so 400 is ample */
   char expanded[400];
   size_t size;

   pci = allocate_element (PRINTF);
   if (pci) {

      /* Expand string here - it's just easier.
       * It should be done in the libca.so
       */
      vsnprintf (expanded, sizeof (expanded), pformat, args);
      va_end (args);

      /* add one for the terminating \0 at end
       */
      size = strlen (expanded) + 1;

      pci->formatted_text = (char *) malloc (size);

      /* Copy expanded string
       */
      memcpy ((void *) pci->formatted_text, &expanded, size);

      load_element (pci);

   }
   return ECA_NORMAL;
}                               /* buffered_printf_handler */


/*------------------------------------------------------------------------------
 */
void initialise_buffered_callbacks ()
{
   linked_list_mutex = epicsMutexCreate ();
   ellInit (&linked_list);
   allocate_fail_count = 0;
}                               /* initialise_buffered_callbacks */


/*------------------------------------------------------------------------------
 */
int number_of_buffered_callbacks ()
{
   /* If initialise_buffered_callbacks has not be called, avoid seg fault and return -1.
    */
   if (!linked_list_mutex) return -1;

   int n;

   epicsMutexLock (linked_list_mutex);
   n = ellCount (&linked_list);
   epicsMutexUnlock (linked_list_mutex);

   return n;
}                               /* number_of_buffered_callbacks */

/*------------------------------------------------------------------------------
 */
void set_multiple_check_limit (const int d)
{
   multiple_check_limit = (d >= 100) ? d : 100;
}                               /* set_multiple_check_limit */

/*------------------------------------------------------------------------------
 */
int  get_multiple_check_limit ()
{
   return multiple_check_limit;
}                               /* get_multiple_check_limit */

/*------------------------------------------------------------------------------
 */
int number_of_discarded_updates ()
{
   int n;

   /* Essentially a diagnostic, so no mutex here.
    */
   n = discard_count;
   discard_count = 0;
   return n;
}                               /* number_of_discarded_updates */


/*------------------------------------------------------------------------------
 * Process callbacks - called from application thread.
 */
int process_buffered_callbacks (const int max)
{
   /* If initialise_buffered_callbacks has not be called, avoid seg fault and return -1.
    */
   if (!linked_list_mutex) return -1;

   Callback_Items *pci = NULL;
   int n;

   if (allocate_fail_count > 0) {
      fprintf (stderr, "*** %s: Allocation failures (%d) \n",
               __FUNCTION__, allocate_fail_count);
      allocate_fail_count = 0;
   }

   n = 0;
   while (1) {

      pci = unload_element ();
      if (pci == NULL) {
         break;
      }

      switch (pci->kind) {

         case CONNECTION:
            application_connection_handler (&pci->cargs);
            break;

         case EVENT:
            application_event_handler (&pci->eargs);
            break;

         case PRINTF:
            application_printf_handler (pci->formatted_text);
            break;

         default:
            fprintf (stderr, "*** %s: Unexpected callback kind: %d \n",
                     __FUNCTION__, pci->kind);
            break;
      }

      /* Free element
       */
      free_element (pci);

      /* Increment counter and test. Test at end of loop in order to process
       * at least one item (if available) regardless of the value of max.
       */
      n++;
      if (n >= max) {
         break;
      }
   }                            /* end loop */

   return n;
}                               /* process_buffered_callbacks */

/*------------------------------------------------------------------------------
 * Discard all outstanding callbacks - called from application thread.
 */
void clear_all_buffered_callbacks ()
{
   /* If initialise_buffered_callbacks has not be called, avoid seg fault and return 0.
    */
   if (!linked_list_mutex) return;

   Callback_Items *pci;

   pci = unload_element ();      /* Get first if it exists */
   while (pci != NULL) {
      free_element (pci);        /* Free element */
      pci = unload_element ();   /* Get next if exists */
   }
}

/* end */
