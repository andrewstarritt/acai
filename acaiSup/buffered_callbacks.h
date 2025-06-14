/* buffered_callbacks.h
 *
 * EPICS buffered callback module for use with Ada, Lazarus and other runtime
 * environments which don't like alien threads, i.e. threads created by 3rd
 * party libraries.  It also provides a buffering mechanism that can be
 * useful even in native C/C++ applications.
 *
 * SPDX-FileCopyrightText: 2005-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 */

/* ---------------------------------------------------------------------------
 * This module provides three functions:
 *
 *   void buffered_connection_handler (struct connection_handler_args args);
 *   void buffered_event_handler (struct event_handler_args args);
 *   int  buffered_printf_handler (const char *pformat, va_list args);
 *
 * These handler functions are not intended to be called directly by the user
 * program, but instead passed as the callback parameter to the relevent
 * functions within the ca library.  Example:
 *
 *  status = ca_array_get_callback (DBR_CTRL_FLOAT, 1, channel_id,
 *                                  buffered_event_handler, NULL);
 *
 * The buffered_xxx_handler functions store a copy of the callback data on a
 * queue. When process_buffered_callbacks is invoked it removes the data from
 * the queue and calls application_xxx_handler, where xxx is one of connection,
 * event or printf. The queue is mutex protected.
 *
 * NOTE: There is ONE queue. If the application is running multiple contexts,
 * then the application_xxx_handler functions must manage the re-direct the
 * response to the appropriate context.
 *
 * The application_connection_handler, the application_event_handler and the
 * application_printf_handler functions must be declared in the user program
 * and made available to the "C" world. These are searched for at link time as
 * opposed to being dynamically registered at run time.
 *
 * Buffered callbacks can be usefull for one or both of the following reasons:
 * a) some language environments, such a Lazarus and Ada, do not like executing
 *    code in threads created outside of the environemnt, i.e. the CA library;
 * b) by processing the data on the queue in the application thread, as opposed
 *    to the CA callback thread that places the data on the queue, all the multi
 *    thread concerns, exclusive access and the like, are confinded to this module.
 *
 * Examples:
 * ---------------------------------------------------------------------------
 * For Ada, the event call back should look something like:
 *
 *    procedure Ada_Event_Handler
 *        (Args : in Channel_Access_Api.Ca_Event_Handler_Args);
 *
 *    pragma Export (C, Ada_Event_Handler, "application_event_handler");
 *
 * ---------------------------------------------------------------------------
 * For Lazarus Pascal, the connection handler would be as follows. Note, while
 * Pascal is normally case insensitive with respect to procedure names, the
 * case of the procedure name IS significant here.
 *
 *   procedure application_connection_handler
 *               (Args : ca_Connection_Handler_Arg_Ptrs); cdecl; export;
 *   begin .... end;
 *
 * ---------------------------------------------------------------------------
 * For C++, one would need this or similar:
 *
 *    extern "C" {
 *       void application_connection_handler (struct connection_handler_args* args);
 *       void application_event_handler (struct event_handler_args* args);
 *       void application_printf_handler (char* formated_text);
 *    }
 *
 *    void application_connection_handler (struct connection_handler_args* args) { .... }
 *    void application_event_handler (struct event_handler_args* args) { .... }
 *    void application_printf_handler (const char* formated_text) { .... }
 *
 * ---------------------------------------------------------------------------
 *
 * NOTE: To aid binding and callback processing in other languages, the
 * buffered callback APIs differ slightly from the native EPICS callback APIs.
 *
 * For the event and connection callback handlers, a pointer to the
 * connection_handler_args or event_handler_args structure is passed to
 * the application handler function, not a copy of the structure.
 *
 * For the printf handler, this unit uses vsprintf to convert the format and
 * va_list args parameters into a plain string.
 *
 */

#ifndef _BUFFERED_CALLBACKS_H_
#define _BUFFERED_CALLBACKS_H_

#include <cadef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These functions are exported by this unit.
 *
 * NOTE: We never call the handlers directly, but do pass the address of these
 * functions as parameters to the relevent functions, i.e. ca_create_channel,
 * ca_array_get_callback, ca_create_subscription, and ca_replace_printf_handler
 * within the ca library.
 */
void buffered_connection_handler (struct connection_handler_args args);
void buffered_event_handler (struct event_handler_args args);
int  buffered_printf_handler (const char* pformat, va_list args);

/* This function should be called once, prior to calling process_buffered_callbacks
 * or the possibility of any callbacks.
 * It creates the internal mutex and initialises the data buffer queue.
 * Under the coveres, this is implemented using an ELLLIST out of ellLib.h
 */
void initialise_buffered_callbacks ();

/* Returns number of currently outstanding buffered callbacks.
 * Returns -1 if initialise_buffered_callbacks has not been called.
 */
int number_of_buffered_callbacks ();

/* Set and get the multiple update check limit.
 * When buffering an update, if the current queue length is greater than or equal
 * to the multiple check limit, default 1000, a search is made for the earliest
 * update for the same channel id, the same update type and the same user argument.
 * If such an update is found, it is removed from the queue.
 * Note: the duplicate check limit is contrained to be >= 100.
 */
void set_multiple_check_limit (const int d);
int  get_multiple_check_limit ();

/* Returns number of discarded duplicate updates.
 * This is a destructive read - i.e. resets the count to zero.
 * Returns 0 if initialise_buffered_callbacks has not been called.
 */
int number_of_discarded_updates ();

/* This function should be called regularly - say every 10-50 mSeconds.
 * It process a maximum of max buffered items. It returns the actual number of
 * callbacks processed (<= max).
 * At least one item is processed, if available, regardless the value of max.
 * Returns -1 if initialise_buffered_callbacks has not been called.
 */
int process_buffered_callbacks (const int max);

/* This function should be called after Channel Accces is no longer required
 * and the EPICS context has been destroyed. It discards and frees the memory
 * associated with all outstanding buffered callbacks.
 * Does nothing if initialise_buffered_callbacks has not been called.
 */
void clear_all_buffered_callbacks ();

#ifdef __cplusplus
}
#endif

#endif                          /* _BUFFERED_CALLBACKS_H_ */
