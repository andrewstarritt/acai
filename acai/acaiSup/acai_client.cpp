/* acai_client.cpp
 *
 * This file is part of the ACAI library. The class was based on the pv_client
 * module developed for the kryten application.
 *
 * Copyright (c) 2013,2014,2015,2016,2017  Andrew C. Starritt
 *
 * This ACAI library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This ACAI library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ACAI library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact details:
 * starritt@netspace.net.au
 * PO Box 3118, Prahran East, Victoria 3181, Australia.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <alarm.h>
#include <alarmString.h>
#include <cadef.h>
#include <caerr.h>
#include <cantProceed.h>
#include <db_access.h>
#include <dbDefs.h>
#include <epicsTime.h>
#include <epicsTypes.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTypes.h>

#include <buffered_callbacks.h>
#include <acai_abstract_client_user.h>
#include <acai_client.h>
#include <acai_private_common.h>

// EPICS timestamp epoch: This is Mon Jan  1 00:00:00 1990 UTC.
//
// This itself is expressed as a system time which represents the number
// of seconds elapsed since 00:00:00 on January 1, 1970, UTC.
//
static const time_t epics_epoch = 631152000;

// Quazi enumeration variables - Channel Access passes back a pointer to one
// of these as user data. It is the address as opposed to the content that is
// important.
// Note: We always get a ref to the client using the args' chanId chid.
//
static int Get = 0;
static int Sub = 0;
static int Put = 0;

// Magic numbers embedded within each ACAI::Client and ACAI::Client::PrivateData object.
// Used as a sainity check when we convert an anonymous pointer to a ACAI::Client
// or ACAI::Client::PrivateData class object.
//
#define MAGIC_NUMBER_C  0x3579ACA1
#define MAGIC_NUMBER_P  0x1234ACA1

// Debug and diagnostic variables.
//
static int debug = 0;

// Useful type neutral numerical macro fuctions.
//
#define ABS(a)             ((a) >= 0  ? (a) : -(a))
#define MIN(a, b)          ((a) <= (b) ? (a) : (b))
#define MAX(a, b)          ((a) >= (b) ? (a) : (b))
#define LIMIT(x,low,high)  (MAX(low, MIN(x, high)))

// Calculates number of items in an array
//
#define ARRAY_LENGTH(xx) ( sizeof (xx) / sizeof (xx [0]) )

#define MINIMUM_BUFFER_SIZE   (sizeof (dbr_string_t))


//==============================================================================
// PRIVATE FUNCTIONS
//==============================================================================
//
static void reportErrorFunc (const int line, const char* function, const char* format, ...)
{
   va_list args;
   char buffer [200];

   va_start (args, format);
   vsnprintf (buffer, sizeof (buffer), format, args);
   va_end (args);

   fprintf (stderr, "ACAI::Client:%d %s: %s\n", line, function, buffer);
}

// Wrapper macros to reportErrorFunc.
// Folds in line number and function name automatically.
//
#define reportError(...) reportErrorFunc (__LINE__, __FUNCTION__, __VA_ARGS__)


//==============================================================================
// ACAI::Client::PrivateData class and methods
//==============================================================================
//
class ACAI::Client::PrivateData {
   int firstMember;         // this together with lastMember define effective size
public:
   explicit PrivateData (ACAI::Client* owner);
   ~PrivateData ();
   void clearBuffer ();     // clears buffer
   const union db_access_val*  updateBuffer (struct event_handler_args& args);

   typedef enum ConnectionStatus {
      csNull = 0,           // channel not in use
      csPending,            // create channel invoked
      csConnected,          // received connect notification
      csDisconnected        // virtual circuit disconnect.
   } ConnectionStatus;

   int magic_number;        // used to verify void* to PrivateData* conversions.

   // Channel Access connection info
   //
   char pv_name [PVNAME_STRINGSZ];
   chid channel_id;             // chid is a pointer
   evid event_id;               // evid is a pointer
   ConnectionStatus connectionStatus;  // do we need this??
   bool lastIsConnected;        // previous value - allows calls to Connection_Bu to be filtered
   ACAI::ReadModes readMode;    // subscription v. single read. v. no read at all
   ACAI::EventMasks eventMask;  // event update tigger specification

   unsigned int priority;       // 0 .. 99
   bool isLongString;
   bool request_element_count_defined;
   unsigned int request_element_count;
   ACAI::ClientFieldType data_request_type;

   bool use_put_callback;       // mode of operation control flag
   bool pending_put_callback;   // indicated waiting for a put callback.

   // Cached channel values
   //
   char channel_host_name [256];
   ACAI::ClientFieldType host_field_type;   // as on host IOC
   unsigned int channel_element_count;      // as on host IOC

   // Meta data (apart from time stamp, returned on first update)
   // Essentially as out of dbr_ctrl_double, dbr_ctrl_enum etc.
   // Use double as this caters for all types (float, long, short etc.)
   //
   int precision;               // number of decimal places
   char units[MAX_UNITS_SIZE];  // units of value
   unsigned char num_states;    // number of strings (was no_str)
   char enum_strings[MAX_ENUM_STATES][MAX_ENUM_STRING_SIZE];    // was strs
   double upper_disp_limit;     // upper limit of graph
   double lower_disp_limit;     // lower limit of graph
   double upper_alarm_limit;
   double upper_warning_limit;
   double lower_warning_limit;
   double lower_alarm_limit;
   double upper_ctrl_limit;     // upper control limit
   double lower_ctrl_limit;     // lower control limit

   // Per update information.
   //
   bool is_first_update;
   unsigned int data_field_size;           // element size  LONG = 4 etc.
   ACAI::ClientFieldType data_field_type;  // as per request - typically same as host_field_type
   unsigned int data_element_count;        // number of elements received (as opposed to
                                           // number on (IOC) server, channel_element_count).
   epicsAlarmCondition status;             // status of value
   epicsAlarmSeverity severity;            // severity of alarm
   epicsTimeStamp	timeStamp;

   // Pointer to actual data as received - we do not decode or unpack the data.
   // We just keep the data blob as we received it.
   // Union for each field type.
   //
   union Data_Values {
      const dbr_string_t* stringRef;
      const dbr_short_t*  shortRef;
      const dbr_float_t*  floatRef;
      const dbr_enum_t*   enumRef;
      const dbr_char_t*   charRef;
      const dbr_long_t*   longRef;
      const dbr_double_t* doubleRef;
      const void*         genericRef;
   };
   union Data_Values dataValues;

   // We use this if/when the data will fit into this buffer, otherwise we use
   // a reference to the call back data - which is okay as already copied
   // once.
   //
   char localBuffer [MINIMUM_BUFFER_SIZE];  // large enough for any scalar

   // When we use a reference to the call back data, this holds a copy of the
   // event_handler_args.dbr member. We need this so that we can free the data
   // on the following update (or on channel close).
   //
   const void* argsDbr;

   // Logical size of buffered data
   // If argsDbr == NULL and logical_data_size > MINIMUM_BUFFER_SIZE we have a problem.
   //
   size_t logical_data_size;

   time_t disconnect_time;       // system time

   // Basic string formatting option. This could be expanded, but that is really
   // the function of any GUI framework and the like, i.e. beyond the scope of
   // this object.
   //
   bool includeUnits;
private:
   ACAI::Client* owner;
   int lastMember;
};

//------------------------------------------------------------------------------
//
ACAI::Client::PrivateData::PrivateData (ACAI::Client* ownerIn)
{
   size_t size;

   // Zeroise all members.
   //
   size = size_t (&this->lastMember) - size_t (&this->firstMember);
   memset (&this->firstMember, 0, size);

   this->owner = ownerIn;

   // Set magic number - used by validate channel id.
   //
   this->magic_number = MAGIC_NUMBER_P;
   this->readMode = ACAI::Subscribe;
   this->eventMask = ACAI::EventMasks (ACAI::EventValue | ACAI::EventAlarm);

   this->connectionStatus = csNull;
   this->channel_id = NULL;
   this->event_id = NULL;

   this->lastIsConnected = false;
   this->isLongString = false;
   this->priority = 10;
   this->request_element_count_defined = false;
   this->request_element_count = 0;
   this->use_put_callback = false;
   this->pending_put_callback = false;

   // Set request typer to default, i.e. request as per native filed type.
   //
   this->data_request_type = ACAI::ClientFieldDefault;

   // We default to the local buffer, big enough to hold a string variable.
   // For 95% of PVs, we will never touch this again.
   //
   this->dataValues.genericRef = &this->localBuffer;
   this->argsDbr = NULL;
   this->logical_data_size = 0;
}

//------------------------------------------------------------------------------
//
ACAI::Client::PrivateData::~PrivateData ()
{
   this->magic_number = 0;
   this->clearBuffer ();
   this->channel_id = NULL;
   this->event_id = NULL;
}

//------------------------------------------------------------------------------
// Free any allocated values data buffer.
// This function is idempotent.
//
void ACAI::Client::PrivateData::clearBuffer ()
{
   if (this->argsDbr) {
      free ((void *) this->argsDbr);
      this->argsDbr = NULL;
   }
   this->dataValues.genericRef = &this->localBuffer;
   this->logical_data_size = 0;
   this->data_element_count = 0;
}

//------------------------------------------------------------------------------
//
const union db_access_val* ACAI::Client::PrivateData::updateBuffer (struct event_handler_args& args)
{
   const union db_access_val* pDbr;
   const void* copyArgsDbr;
   void* valuesPtr;
   size_t length;

   copyArgsDbr = this->argsDbr;      // Save what we currently are referencing, if anything; and
   this->argsDbr = NULL;             // ensure no dangling ref.

   // Reference to new data values and length, both sans meta data.
   //
   valuesPtr = dbr_value_ptr (args.dbr, args.type);
   length = dbr_value_size [args.type] * args.count;

   if (length <= sizeof (this->localBuffer)) {
      // Data small enough to fit into out local buffer - just copy.
      // The buffered callback module will free this memory.
      //
      memcpy ((void*) this->dataValues.genericRef, valuesPtr, length);
      this->dataValues.genericRef = &this->localBuffer;
      pDbr = (union db_access_val *) args.dbr;

   } else {
      // Data tooo big for local buffer, just "highjack" args.dbr. But we must
      // clear args.dbr so that buffered callback does NOT attempt to free this
      // data.
      //
      this->argsDbr = args.dbr;         // copy item ref.
      args.dbr = NULL;                  // clear args item ref
      this->dataValues.genericRef = valuesPtr;
      pDbr = (union db_access_val *) this->argsDbr;
   }

   // Lastly free our old buffer if we had one.
   //
   if (copyArgsDbr) {
      free ((void*) copyArgsDbr);
   }

   return pDbr;
}


//==============================================================================
// ACAI::Client class methods
//==============================================================================
//
ACAI::Client::Client (const ClientString& pvName)
{
   // Create the private data.
   //
   this->pd = new PrivateData (this);

   // Ensure callbacks are NULL.
   //
   this->connectionUpdateEventHandler = NULL;
   this->dataUpdateEventHandler = NULL;
   this->putCallbackEventHandler = NULL;

   this->registeredUsers.clear ();

   // Clear user tags - after this, we ignore these.
   //
   this->userTag = 0;
   this->userRefTag = NULL;
   this->userStringTag = "";

   // Set magic number - used by validate channel id.
   //
   this->magic_number = MAGIC_NUMBER_C;

   this->setPvName (pvName);
}

//------------------------------------------------------------------------------
//
ACAI::Client::~Client ()
{
   this->unsubscribeChannel ();
   this->closeChannel ();
   this->removeClientFromAllUserLists ();

   this->magic_number = 0;

   delete this->pd;
   this->pd = NULL;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setPvName (const ClientString& pvName, const bool doImmediateReopen)
{
   const char* c_string_name;

   // We store the PV name as a traditional C string, as this is the format
   // required by the Channel Access API.
   //
   c_string_name = pvName.c_str ();

   // snprintf ensures pv_name is null terminated.
   //
   snprintf (this->pd->pv_name, sizeof (this->pd->pv_name), "%s", c_string_name);

   if (doImmediateReopen) {
      this->reopenChannel ();
   }
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::pvName () const
{
   return ClientString (this->pd->pv_name);
}

//------------------------------------------------------------------------------
//
const char* ACAI::Client::cPvName () const
{
   return this->pd->pv_name;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setDataRequestType (const ACAI::ClientFieldType fieldType)
{
   if (fieldType != ACAI::ClientFieldNO_ACCESS) {
      this->pd->data_request_type = fieldType;
   }
}

//------------------------------------------------------------------------------
//
ACAI::ClientFieldType  ACAI::Client::dataRequestType () const
{
   return this->pd->data_request_type;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setRequestCount (const unsigned int number)
{
   this->pd->request_element_count = number;
   this->pd->request_element_count_defined = true;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::clearRequestCount ()
{
   this->pd->request_element_count = 0;
   this->pd->request_element_count_defined = false;
}

//------------------------------------------------------------------------------
//
unsigned int ACAI::Client::requestCount (bool& isDefined) const
{
   isDefined = this->pd->request_element_count_defined;
   return this->pd->request_element_count;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setPriority (const unsigned int priority)
{
   this->pd->priority = LIMIT (priority, 0, 99);
}


unsigned int ACAI::Client::priority () const
{
   return this->pd->priority;
}


//------------------------------------------------------------------------------
//
void ACAI::Client::setLongString (const bool isLongStringIn)
{
   this->pd->isLongString = isLongStringIn;
}


bool ACAI::Client::isLongString () const
{
   return this->pd->isLongString;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setReadMode (const ACAI::ReadModes readModeIn)
{
   this->pd->readMode = readModeIn;
}

ACAI::ReadModes ACAI::Client::readMode () const
{
   return this->pd->readMode;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setEventMask (const ACAI::EventMasks eventMaskIn)
{
   this->pd->eventMask = eventMaskIn;
}

ACAI::EventMasks ACAI::Client::eventMask () const
{
   return this->pd->eventMask;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setUsePutCallback (const bool usePutCallbackIn)
{
   this->pd->use_put_callback = usePutCallbackIn;
}

bool ACAI::Client::usePutCallback () const
{
   return this->pd->use_put_callback;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::isPendingPutCallback () const
{
   return this->pd->pending_put_callback;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::clearPendingPutCallback ()
{
   if (this->pd->pending_put_callback) {
      this->pd->pending_put_callback = false;
      this->callPutCallbackNotifcation (false);
   }
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::openChannel ()
{
   bool result = false;
   int status;

   // TO DECIDE: close if already open ?? or dis-allow ??

   if (strlen (this->pd->pv_name) > 0) {

      status = ca_create_channel (this->pd->pv_name,
                                  buffered_connection_handler,
                                  this,     // user private
                                  this->pd->priority,
                                  &this->pd->channel_id);

      if (status == ECA_NORMAL) {
         this->pd->connectionStatus = PrivateData::csPending;
         result = true;
      } else {
         reportError ("ca_create_channel (%s) failed (%s, %d)", this->pd->pv_name,
                      ca_message (status), status);
         result = false;
      }
   } else {
      result = true;   // just ignore empty name - not deemed a failure.
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::closeChannel ()
{
   int status;

   // The unsubscribe function checks if we are subscribed.
   //
   this->unsubscribeChannel ();

   // Close channel iff needs be.
   //
   if (this->pd->channel_id) {
      status = ca_clear_channel (this->pd->channel_id);
      if (status != ECA_NORMAL) {
         reportError ("ca_clear_channel (%s) failed (%s)",
                      this->pd->pv_name, ca_message (status));
      }

      this->pd->channel_id = NULL;
   }

   this->pd->connectionStatus = PrivateData::csNull;
   this->pd->pending_put_callback = false;

   // Free any allocated values buffer.
   //
   this->pd->clearBuffer ();
   this->callConnectionUpdate ();
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::reopenChannel ()
{
   this->closeChannel ();
   return this->openChannel ();
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::reReadChannel () {
   bool result = false;

   if (this->isConnected ()) {
      result = this->readSubscribeChannel (ACAI::SingleRead);
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::isConnected () const
{
   return (this->pd->connectionStatus == PrivateData::csConnected);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::dataIsAvailable () const
{
   return (this->isConnected ()) &&
         (this->pd->dataValues.genericRef) &&
         (this->pd->logical_data_size > 0);
}

//------------------------------------------------------------------------------
//
ACAI::ClientFloating ACAI::Client::getFloating (unsigned int index) const
{
   double result = 0.0;

   if ((this->dataIsAvailable ()) &&
       (index < this->pd->data_element_count)) {

      switch (this->pd->data_field_type) {
         case ACAI::ClientFieldSTRING:
            result = (ClientFloating) atof (this->pd->dataValues.stringRef[index]);
            break;
         case ACAI::ClientFieldSHORT:
            result = (ClientFloating) this->pd->dataValues.shortRef[index];
            break;
         case ACAI::ClientFieldFLOAT:
            result = (ClientFloating) this->pd->dataValues.floatRef[index];
            break;
         case ACAI::ClientFieldENUM:
            result = (ClientFloating) this->pd->dataValues.enumRef[index];
            break;
         case ACAI::ClientFieldCHAR:
            result = (ClientFloating) this->pd->dataValues.charRef[index];
            break;
         case ACAI::ClientFieldLONG:
            result = (ClientFloating) this->pd->dataValues.longRef[index];
            break;
         case ACAI::ClientFieldDOUBLE:
            result = (ClientFloating) this->pd->dataValues.doubleRef[index];
            break;
         default:
            result = 0.0;
            break;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientInteger ACAI::Client::getInteger (unsigned int index) const
{
   int result = 0;

   if ((this->dataIsAvailable ()) &&
       (index < this->pd->data_element_count)) {

      switch (this->pd->data_field_type) {
         case ACAI::ClientFieldSTRING:
            result = (ClientInteger) atoi (this->pd->dataValues.stringRef[index]);
            break;
         case ACAI::ClientFieldSHORT:
            result = (ClientInteger) this->pd->dataValues.shortRef[index];
            break;
         case ACAI::ClientFieldFLOAT:
            result = (ClientInteger) this->pd->dataValues.floatRef[index];
            break;
         case ACAI::ClientFieldENUM:
            result = (ClientInteger) this->pd->dataValues.enumRef[index];
            break;
         case ACAI::ClientFieldCHAR:
            result = (ClientInteger) this->pd->dataValues.charRef[index];
            break;
         case ACAI::ClientFieldLONG:
            result = (ClientInteger) this->pd->dataValues.longRef[index];
            break;
         case ACAI::ClientFieldDOUBLE:
            result = (ClientInteger) this->pd->dataValues.doubleRef[index];
            break;
         default:
            result = 0;
            break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::getString (unsigned int index) const
{
   char append_units[MAX_UNITS_SIZE + 2];
   char format[20];
   int enum_state;
   int p;
   ACAI::ClientString result;

   result = ClientString ("");     // set default

   if (!this->dataIsAvailable ()) {
      return result;
   }

   // Is this PV to be treated as a long string?
   // If so, only the default 'element 0' yields the string.
   //
   if (this->processingAsLongString ()) {
      if (index == 0) {
         const char* text = (const char *) this->pd->dataValues.charRef;
         result = ACAI::limitedAssign (text, this->pd->data_element_count);
      } else {
         result = "";
      }
      return result;
   }

   if ((this->pd->includeUnits) && (strlen (this->pd->units) > 0)) {
      snprintf (append_units, sizeof (append_units), " %s",
                this->pd->units);
   } else {
      append_units[0] = '\0';
   }

   // Regular string processing.
   //
   if (index < this->pd->data_element_count) {

      switch (this->pd->data_field_type) {
         case ACAI::ClientFieldSTRING:
            // Do not copy more than MAX_STRING_SIZE characters.
            //
            result = ACAI::limitedAssign (this->pd->dataValues.stringRef [index], MAX_STRING_SIZE);
            break;

         case ACAI::ClientFieldCHAR:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            result = ACAI::csnprintf (50, "%d%s", this->getInteger (index), append_units);
            break;

         case ACAI::ClientFieldENUM:
            enum_state = this->getInteger (index);
            result = this->getEnumeration (enum_state);
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            // Set up the format string.
            p = this->precision ();
            p = LIMIT (p, 0, 15);
            snprintf (format, sizeof (format), "%%.%df%%s", p);
            result = ACAI::csnprintf (50, format, this->getFloating (index), append_units);
            break;

         default:
            result = "";
            break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientFloatingArray ACAI::Client::getFloatingArray () const
{
   ACAI::ClientFloatingArray result;

   if (this->dataIsAvailable ()) {
      const unsigned int number = this->dataElementCount ();

      result.reserve ((int) number);
      for (unsigned int j = 0; j < number; j++) {
         result.push_back (this->getFloating (j));
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientIntegerArray ACAI::Client::getIntegerArray () const
{
   ACAI::ClientIntegerArray result;

   if (this->dataIsAvailable ()) {
      const unsigned int number = this->dataElementCount ();

      result.reserve ((int) number);
      for (unsigned int j = 0; j < number; j++) {
         result.push_back (this->getInteger (j));
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientStringArray ACAI::Client::getStringArray () const
{
   ACAI::ClientStringArray result;

   if (this->dataIsAvailable ()) {
      const unsigned int number = this->dataElementCount ();

      // TODO - long String check

      result.reserve ((int) number);
      for (unsigned int j = 0; j < number; j++) {
         result.push_back (this->getString (j));
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putData (const int dbf_type, const unsigned long  count, const void* dataPtr)
{
   const chtype type = chtype (dbf_type);
   int status;

   // Are we connected/do we have a channel id ?
   //
   if (!this->isConnected () || !this->pd->channel_id) {
      return false;
   }

   // Are we using put callback on this channel ?
   //
   if (this->pd->use_put_callback) {
      // Yes - are we already waiting for a callback ?
      //
      if (this->pd->pending_put_callback) {
         reportError ("putData (%s) write inhibited - pending put callback", this->pd->pv_name);
         return false;
      }

      status = ca_array_put_callback (type, count, this->pd->channel_id,
                                      dataPtr, buffered_event_handler, &Put);

      // If write was successful, set pending flag.
      //
      this->pd->pending_put_callback = (status == ECA_NORMAL);

   } else {
      // No call back - just a regular put.
      //
      status = ca_array_put (type, count, this->pd->channel_id, dataPtr);
   }

   // Convert to a boolean result.
   //
   return  (status == ECA_NORMAL);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putFloating (const ACAI::ClientFloating value)
{
   const dbr_double_t dbr_value = (dbr_double_t) value;
   return this->putData (DBF_DOUBLE, 1, &dbr_value);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putInteger (const ACAI::ClientInteger value)
{
   const dbr_long_t dbr_value = (dbr_long_t) value;
   return this->putData (DBF_LONG, 1, &dbr_value);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putString (const ACAI::ClientString& value)
{
   // Convert and truncate ClientString to basic c string.
   //
   dbr_string_t dbr_value;
   snprintf (dbr_value, sizeof (dbr_value), "%s", value.c_str ());
   return this->putData (DBF_STRING, 1, dbr_value);
}


//------------------------------------------------------------------------------
//
bool ACAI::Client::putFloatingArray (const ACAI::ClientFloating* valueArray, const unsigned int count)
{
   // We know ACAI::ClientFloating is double - no element by element conversion required.
   return this->putData (DBF_DOUBLE, count, valueArray);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putFloatingArray (const ACAI::ClientFloatingArray& valueArray)
{
   const unsigned int count = (unsigned int) valueArray.size ();
   const ACAI::ClientFloating* podPtr = &valueArray [0];  // convert to "Plain Old Data" type.

   return this->putFloatingArray (podPtr, count);
}


//------------------------------------------------------------------------------
//
bool ACAI::Client::putIntegerArray (const ACAI::ClientInteger* valueArray, const unsigned int count)
{
   // We know ACAI::ClientInteger is long - no element by element conversion required.
   return this->putData (DBF_LONG, count, valueArray);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putIntegerArray (const ACAI::ClientIntegerArray& valueArray)
{
   const unsigned int count = (unsigned int) valueArray.size ();
   const ACAI::ClientInteger* podPtr = &valueArray [0];

   return this->putIntegerArray (podPtr, count);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putStringArray (const ACAI::ClientString* valueArray, const unsigned int count)
{
   bool result = false;

   // Convert and truncate ClientStrings array to basic c strings.
   //
   dbr_string_t* buffer = NULL;
   buffer = (dbr_string_t *) calloc ((size_t) count, sizeof (dbr_string_t));
   if (buffer) {
      for (unsigned int j = 0; j < count; j++) {
         snprintf (buffer [j * sizeof (dbr_string_t)], sizeof (dbr_string_t), "%s",
                   valueArray [j].c_str ());
      }
      result = this->putData (DBF_STRING, count, buffer);
      free ((void *) buffer);
   } else {
      // report error
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::putStringArray (const ACAI::ClientStringArray& valueArray)
{
   const unsigned int count = (unsigned int) valueArray.size ();
   const ACAI::ClientString* podPtr = &valueArray [0];

   return this->putStringArray (podPtr, count);
}

//------------------------------------------------------------------------------
//
int ACAI::Client::enumerationStatesCount () const
{
   int result;

   if (this->dataFieldType () == ClientFieldENUM) {
      // Is this an alarm status (.STAT) PV ?
      //
      if (this->isAlarmStatusPv ()) {
         // Yes - use alarm status strings - there are more than the 16
         //
         result = ALARM_NSTATUS;
      } else {
         // No - use channel access values.
         //
         result = this->pd->num_states;
      }
   } else {
      result = 0;
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::getEnumeration (int state) const
{
   ACAI::ClientString result;

   // Set default.
   //
   result = ACAI::csnprintf (12, "#%d", state);

   if (this->dataFieldType () == ClientFieldENUM) {

      int n = this->enumerationStatesCount ();

      // Is specified state in range ?
      //
      if ((state >= 0) && (state < n)) {
         const char* source;

         // Is this an alarm status PV ? i.e. {recordname}.STAT
         //
         if (this->isAlarmStatusPv ()) {
            // Yes - use alarm status strings - there are more than the 16
            // provided by the CA library.
            //
            source = epicsAlarmConditionStrings [state];

         } else {
            // No - use channel access values.
            //
            source = this->pd->enum_strings [state];
         }

         // If the enum values are at max size, there is no null end-of-string
         // character at the end of the value.  Do not run into next enum value
         // Do not copy more than MAX_ENUM_STRING_SIZE characters.
         //
         result = ACAI::limitedAssign (source, MAX_ENUM_STRING_SIZE);
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientStringArray ACAI::Client::getEnumerationStates () const
{
   ACAI::ClientStringArray result;
   int j;
   int n;

   result.clear ();
   n = this->enumerationStatesCount ();
   for (j = 0; j < n; j++) {
      result.push_back (this->getEnumeration (j));
   }
   return result;
}

//------------------------------------------------------------------------------
//
size_t ACAI::Client::rawDataSize () const
{
   return this->pd->logical_data_size;
}

//------------------------------------------------------------------------------
//
size_t ACAI::Client::getRawData (void* dest, const size_t size,
                                 const size_t offset) const
{
   size_t count = 0;
   if (this->dataIsAvailable ()) {
      if (offset < this->pd->logical_data_size) {
         const size_t available_size = this->pd->logical_data_size - offset;
         count = MIN (size, available_size);
         memcpy (dest, (this->pd->dataValues.charRef + offset), count);
      }
   }
   return count;
}

//------------------------------------------------------------------------------
//
const void* ACAI::Client::rawDataPointer (size_t& count,
                                          const size_t offset) const
{
   const void* result = NULL;
   count = 0;
   if (this->dataIsAvailable ()) {
      if (offset < this->pd->logical_data_size) {
         const size_t available_size = this->pd->logical_data_size - offset;
         count = available_size;
         result = (const void*) (this->pd->dataValues.charRef + offset);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setIncludeUnits (const bool value)
{
   this->pd->includeUnits = value;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::includeUnits () const
{
   return this->pd->includeUnits;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::alarmStatusImage () const
{
   int n;
   n = (int) this->alarmStatus ();
   if ((n >= 0) && (n < ALARM_NSTATUS)) {
      return ACAI::ClientString (epicsAlarmConditionStrings[n]);
   }

   return ACAI::csnprintf (40, "unknown status %d", n);
}


//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::alarmSeverityImage () const
{
   int n;
   n = (int) this->alarmSeverity ();
   if ((n >= 0) && (n < ALARM_NSEV)) {
      return ACAI::ClientString (epicsAlarmSeverityStrings[n]);
   }
   if (n == ClientDisconnected) {
      return ACAI::ClientString ("DISCONNECTED");
   }

   return ACAI::csnprintf (40, "unknown severity %d", n);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::readAccess () const
{
   return ca_read_access (this->pd->channel_id);
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::writeAccess () const
{
   return ca_write_access (this->pd->channel_id);
}

//------------------------------------------------------------------------------
//
time_t ACAI::Client::utcTime (int* nanoSecOut) const
{
   if (nanoSecOut) {
      *nanoSecOut = this->pd->timeStamp.nsec;
   }
   return this->pd->timeStamp.secPastEpoch + epics_epoch;
}

//------------------------------------------------------------------------------
//
ACAI::ClientTimeStamp ACAI::Client::timeStamp () const
{
   ACAI::ClientTimeStamp result;

   result.secPastEpoch = this->pd->timeStamp.secPastEpoch;
   result.nsec         = this->pd->timeStamp.nsec;
   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::Client::utcTimeImage (const int precision) const
{
   static const int scale [10] = {
      1000000000, 100000000, 10000000, 1000000,
      100000, 10000, 1000, 100, 10, 1
   };

   ACAI::ClientString result;
   int nano_sec;
   struct tm bt;
   char text [40];

   // Make maybe uninitialised warning go away.
   //
   bt.tm_year = bt.tm_mon = bt.tm_mday = bt.tm_hour = bt.tm_min = bt.tm_sec = 0;

   // Form broken-down UTC time bt
   //
   const time_t utc = this->utcTime (&nano_sec);
   gmtime_r (&utc, &bt);

   // In broken-down time, tm_year is the number of years since 1900,
   // and January is month 0.
   //
   snprintf (text, 40, "%04d-%02d-%02d %02d:%02d:%02d",
             1900 + bt.tm_year, 1 + bt.tm_mon, bt.tm_mday,
             bt.tm_hour, bt.tm_min, bt.tm_sec);

   result = text;

   if (precision > 0) {
      char format[8];
      char fraction[16];

      const int p = MIN (precision, 9);
      sprintf (format, ".%%0%dd", p);
      const int f = nano_sec / scale[p];
      sprintf (fraction, format, f);
      result.append (fraction);
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::isAlarmStatusPv () const
{
   ACAI::ClientString pvName = this->pvName ();
   int l = pvName.length ();

   return (l >= 5) && (pvName.substr (l - 5, 5) == ".STAT");
}

//------------------------------------------------------------------------------
//
bool ACAI::Client::processingAsLongString () const
{
   ACAI::ClientString pvName = this->pvName ();
   int l = pvName.length ();

   return (this->pd->host_field_type == ACAI::ClientFieldCHAR) &&
          (this->pd->isLongString || (l >= 1 && this->pvName() [l - 1] == '$'));
}

//------------------------------------------------------------------------------
// Get initial data and subscribe for updates.
//
bool ACAI::Client::readSubscribeChannel (const ACAI::ReadModes readMode)
{
   static const unsigned long default_max_array_size = 16384;

   unsigned long count;
   ACAI::ClientFieldType actualRequestType;
   chtype initial_type;
   chtype update_type;
   unsigned long size;
   char *env_var;
   unsigned long max_array_size;
   unsigned long meta_data_size;
   unsigned long truncated;
   int status;

   count = this->pd->channel_element_count;
   if (count == 0) {
      reportError ("element count (%s) is zero", this->pd->pv_name);
      return false;
   }

   // If user has specified number of elements then honor this.
   //
   if (this->pd->request_element_count_defined) {
      count = MIN (count, this->pd->request_element_count);
   }

   // Determine initial buffer request type and subscription buffer
   // request type, based on the request field type.
   //
   actualRequestType = this->pd->data_request_type;
   if (actualRequestType == ACAI::ClientFieldDefault) {
      // We are going with the default.
      //
      actualRequestType = this->pd->host_field_type;
   }

   switch (actualRequestType) {

      case ACAI::ClientFieldSTRING:
         initial_type = DBR_STS_STRING;
         update_type = DBR_TIME_STRING;
         size = sizeof (dbr_string_t);
         break;

      case ACAI::ClientFieldSHORT:
         initial_type = DBR_CTRL_SHORT;
         update_type = DBR_TIME_SHORT;
         size = sizeof (dbr_short_t);
         break;

      case ACAI::ClientFieldFLOAT:
         initial_type = DBR_CTRL_FLOAT;
         update_type = DBR_TIME_FLOAT;
         size = sizeof (dbr_float_t);
         break;

      case ACAI::ClientFieldENUM:
         initial_type = DBR_CTRL_ENUM;
         update_type = DBR_TIME_ENUM;
         size = sizeof (dbr_enum_t);
         break;

      case ACAI::ClientFieldCHAR:
         initial_type = DBR_CTRL_CHAR;
         update_type = DBR_TIME_CHAR;
         size = sizeof (dbr_char_t);
         break;

      case ACAI::ClientFieldLONG:
         initial_type = DBR_CTRL_LONG;
         update_type = DBR_TIME_LONG;
         size = sizeof (dbr_long_t);
         break;

      case ACAI::ClientFieldDOUBLE:
         initial_type = DBR_CTRL_DOUBLE;
         update_type = DBR_TIME_DOUBLE;
         size = sizeof (dbr_double_t);
         break;

      default:
         reportError ("field type (%s) is invalid (%d)", this->pd->pv_name,
                      (int) actualRequestType);
         return false;
   }

   max_array_size = default_max_array_size;

   // Attempt to read EPICS_CA_MAX_ARRAY_BYTES environment variable.
   //
   env_var = getenv ("EPICS_CA_MAX_ARRAY_BYTES");
   if (env_var) {
      status = sscanf (env_var, "%ld", &max_array_size);
      if (status == 1) {
         // Limit to no less than default size
         //
         max_array_size = MAX (max_array_size, default_max_array_size);
      } else {
         reportError ("EPICS_CA_MAX_ARRAY_BYTES %s is non numeric", env_var);
      }
   }

   // We 'know' that the initial request meta data size is larger
   // than the update meta data size.
   //
   meta_data_size = dbr_size_n (initial_type, 1);

   // Check if Max_Array_Size, i.e. ${EPICS_CA_MAX_ARRAY_BYTES}, is
   // large enough to handle the request.
   //
   if ((meta_data_size + (count * size)) >= max_array_size) {
      truncated = (max_array_size - meta_data_size) / size;

      reportError ("PV (%s) request count truncated from %ld to %ld elements",
                   this->pd->pv_name, count, truncated);
      reportError ("Effective EPICS_CA_MAX_ARRAY_BYTES = %ld",
                   max_array_size);

      count = truncated;
   }

   // Read data together with all meta data.
   //
   if ((readMode == ACAI::SingleRead) || (readMode == ACAI::Subscribe)) {
      status = ca_array_get_callback (initial_type, count, this->pd->channel_id,
                                      buffered_event_handler, &Get);

      if (status != ECA_NORMAL) {
         reportError ("ca_array_get_callback (%s) failed (%s)", this->pd->pv_name,
                      ca_message (status));
         return false;
      }
   }

   // If read fails - so will subscribe.
   //
   if (readMode == ACAI::Subscribe) {
      // ... and now subscribe for time stamped data updates as well.
      //
      status = ca_create_subscription (update_type, count, this->pd->channel_id,
                                       this->pd->eventMask, buffered_event_handler,
                                       &Sub, &this->pd->event_id);

      if (status != ECA_NORMAL) {
         reportError ("ca_create_subscription (%s) failed (%s)",
                      this->pd->pv_name, ca_message (status));
         return false;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::unsubscribeChannel ()
{
   int status;

   // Unsubscribe iff needs be.
   //
   if (this->pd->event_id) {
      status = ca_clear_subscription (this->pd->event_id);
      if (status != ECA_NORMAL) {
         reportError ("ca_clear_subscription (%s) failed (%s)",
                      this->pd->pv_name, ca_message (status));
      }
      this->pd->event_id = NULL;

      // Save discconection time
      //
      time (&this->pd->disconnect_time);
   }
}

//------------------------------------------------------------------------------
// Processes received data
//
// Arguments passed to event handlers and get/put call-back handlers.
//
// The status field below is the CA ECA_XXX status of the requested
// operation which is saved from when the operation was attempted in the
// server and copied back to the clients call back routine.
// If the status is not ECA_NORMAL then the dbr pointer will be NULL
// and the requested operation can not be assumed to be successful.
//
//   typedef struct event_handler_args {
//       void            *usr;    -- user argument supplied with request
//       chanId          chid;    -- channel id
//       long            type;    -- the type of the item returned
//       long            count;   -- the element count of the item returned
//       READONLY void   *dbr;    -- a pointer to the item returned
//       int             status;  -- ECA_XXX status from the server
//   } evargs;
//
//
void ACAI::Client::updateHandler (struct event_handler_args& args)
{
   // These two used as 'globals' in the following macro functions.
   //
   PrivateData* tpd = this->pd;      // alias
   const union db_access_val* pDbr;

   // Local macro "functons" that make use of naming regularity
   // The inital updates have no time - so we use time now.
   //
#define ASSIGN_STATUS(from)                                                \
   tpd->data_element_count = args.count;                                   \
   tpd->status = (epicsAlarmCondition) from.status;                        \
   tpd->severity = (epicsAlarmSeverity) from.severity;                     \
   tpd->timeStamp = epicsTime::getCurrent ();


   // Convert EPICS time to system time.
   // EPICS is number seconds since 01-Jan-1990 where as
   // System time is number seconds since 01-Jan-1970.
   //
#define ASSIGN_STATUS_AND_TIME(from)                                       \
   tpd->data_element_count = args.count;                                   \
   tpd->status = (epicsAlarmCondition) from.status;                        \
   tpd->severity = (epicsAlarmSeverity) from.severity;                     \
   tpd->timeStamp = from.stamp


#define ASSIGN_META_DATA(from, prec)                                       \
   tpd->precision = prec;                                                  \
   strncpy (tpd->units, pDbr->cfltval.units, sizeof (tpd->units) - 1);     \
   tpd->num_states = 0;                                                    \
   tpd->upper_disp_limit    = (double) from.upper_disp_limit;              \
   tpd->lower_disp_limit    = (double) from.lower_disp_limit;              \
   tpd->upper_alarm_limit   = (double) from.upper_alarm_limit;             \
   tpd->upper_warning_limit = (double) from.upper_warning_limit;           \
   tpd->lower_warning_limit = (double) from.lower_warning_limit;           \
   tpd->lower_alarm_limit   = (double) from.lower_alarm_limit;             \
   tpd->upper_ctrl_limit    = (double) from.upper_ctrl_limit;              \
   tpd->lower_ctrl_limit    = (double) from.lower_ctrl_limit;


#define CLEAR_META_DATA                                                    \
   tpd->precision = 0;                                                     \
   tpd->units[0] = '\0';                                                   \
   tpd->num_states = 0;                                                    \
   tpd->upper_disp_limit    = 0.0;                                         \
   tpd->lower_disp_limit    = 0.0;                                         \
   tpd->upper_alarm_limit   = 0.0;                                         \
   tpd->upper_warning_limit = 0.0;                                         \
   tpd->lower_warning_limit = 0.0;                                         \
   tpd->lower_alarm_limit   = 0.0;                                         \
   tpd->upper_ctrl_limit    = 0.0;                                         \
   tpd->lower_ctrl_limit    = 0.0;

   size_t length;

   if (tpd->connectionStatus != PrivateData::csConnected) {
      reportError  ("%s: connection status is not csConnected (%d), type=%s (%ld)",
                    tpd->pv_name, (int) tpd->connectionStatus,
                    ACAI::Client::dbRequestTypeImage (args.type), args.type);
      return;
   }

   if (!dbr_type_is_valid (args.type)) {
      reportError ("%s: invalid dbr type %s (%ld)", tpd->pv_name,
                   ACAI::Client::dbRequestTypeImage (args.type), args.type);
      return;
   }

   // Get length of value data, i.e. exclude meta data.
   //
   length = dbr_value_size [args.type] * args.count;

   if (length <= 0) {
      reportError ("%s: zero/negative (%ld) length data for dbr type %s (%ld)",
                   tpd->pv_name, args.count, ACAI::Client::dbRequestTypeImage (args.type),
                   args.type);
      return;
   }

   // Save data attributes
   //
   tpd->logical_data_size = length;
   tpd->data_field_size = dbr_value_size [args.type];

   // Sorts out buffering and updates this->dataValues.genericRef
   //
   pDbr = tpd->updateBuffer (args);

   // Maybe the following should move to updateBuffer as well.
   //
   switch (args.type) {

      /// Control updates set all meta data plus initial values

      case DBR_STS_STRING:
         tpd->data_field_type = ClientFieldSTRING;
         ASSIGN_STATUS (pDbr->cstrval);
         CLEAR_META_DATA;
         break;

      case DBR_CTRL_SHORT:
         tpd->data_field_type = ClientFieldSHORT;
         ASSIGN_STATUS (pDbr->cshrtval);
         ASSIGN_META_DATA (pDbr->cshrtval, 0);
         break;

      case DBR_CTRL_FLOAT:
         tpd->data_field_type = ClientFieldFLOAT;
         ASSIGN_STATUS (pDbr->cfltval);
         ASSIGN_META_DATA (pDbr->cfltval, pDbr->cfltval.precision);
         break;

      case DBR_CTRL_ENUM:
         tpd->data_field_type = ClientFieldENUM;
         ASSIGN_STATUS (pDbr->cenmval);
         CLEAR_META_DATA;
         tpd->num_states = pDbr->cenmval.no_str;

         // Set up sensible display/control upper limit.
         //
         if (this->isAlarmStatusPv ()) {
            tpd->upper_disp_limit = ALARM_NSTATUS - 1.0;
            tpd->upper_ctrl_limit = ALARM_NSTATUS - 1.0;
         } else {
            tpd->upper_disp_limit = tpd->num_states - 1.0;
            tpd->upper_ctrl_limit = tpd->num_states - 1.0;
         }

         memcpy (tpd->enum_strings, pDbr->cenmval.strs,
                 sizeof (tpd->enum_strings));
         break;

      case DBR_CTRL_CHAR:
         tpd->data_field_type = ClientFieldCHAR;
         ASSIGN_STATUS (pDbr->cchrval);
         ASSIGN_META_DATA (pDbr->cchrval, 0);
         break;

      case DBR_CTRL_LONG:
         tpd->data_field_type = ClientFieldLONG;
         ASSIGN_STATUS (pDbr->clngval);
         ASSIGN_META_DATA (pDbr->clngval, 0);
         break;

      case DBR_CTRL_DOUBLE:
         tpd->data_field_type = ClientFieldDOUBLE;
         ASSIGN_STATUS (pDbr->cdblval);
         ASSIGN_META_DATA (pDbr->cdblval, pDbr->cdblval.precision);
         break;

         /// Time updates values [count], time, severity and status

      case DBR_TIME_STRING:
         tpd->data_field_type = ClientFieldSTRING;
         ASSIGN_STATUS_AND_TIME (pDbr->tstrval);
         break;

      case DBR_TIME_SHORT:
         tpd->data_field_type = ClientFieldSHORT;
         ASSIGN_STATUS_AND_TIME (pDbr->tshrtval);
         break;

      case DBR_TIME_FLOAT:
         tpd->data_field_type = ClientFieldFLOAT;
         ASSIGN_STATUS_AND_TIME (pDbr->tfltval);
         break;

      case DBR_TIME_ENUM:
         tpd->data_field_type = ClientFieldENUM;
         ASSIGN_STATUS_AND_TIME (pDbr->tenmval);
         break;

      case DBR_TIME_CHAR:
         tpd->data_field_type = ClientFieldCHAR;
         ASSIGN_STATUS_AND_TIME (pDbr->tchrval);
         break;

      case DBR_TIME_LONG:
         tpd->data_field_type = ClientFieldLONG;
         ASSIGN_STATUS_AND_TIME (pDbr->tlngval);
         break;

      case DBR_TIME_DOUBLE:
         tpd->data_field_type = ClientFieldDOUBLE;
         ASSIGN_STATUS_AND_TIME (pDbr->tdblval);
         break;

      default:
         tpd->data_field_type = ClientFieldNO_ACCESS;
         reportError ("(%s): unexpected buffer type %ld", tpd->pv_name, args.type);
         return;
   }

#undef ASSIGN_STATUS
#undef ASSIGN_STATUS_AND_TIME
#undef ASSIGN_META_DATA
#undef CLEAR_META_DATA

   this->callDataUpdate (tpd->is_first_update);
   tpd->is_first_update = false;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::connectionHandler (struct connection_handler_args& args)
{
   switch (args.op) {

      case CA_OP_CONN_UP:
         if (debug >= 4) {
            reportError ("PV connected %s", this->pd->pv_name);
         }

         this->pd->connectionStatus = PrivateData::csConnected;
         // Relies on our definitions being consistant.
         this->pd->host_field_type = (ACAI::ClientFieldType) ca_field_type (this->pd->channel_id);
         this->pd->channel_element_count = ca_element_count (this->pd->channel_id);

         // Copy host name.
         //
         ca_get_host_name (this->pd->channel_id, this->pd->channel_host_name,
                           sizeof (this->pd->channel_host_name));

         this->pd->data_element_count = 0;                // no data yet
         this->pd->is_first_update = true;                // initial request.
         this->readSubscribeChannel (this->pd->readMode); // read and optionally subscribe.
         this->callConnectionUpdate ();
         break;

      case CA_OP_CONN_DOWN:
         if (debug >= 4) {
            reportError ("PV disconnected %s", this->pd->pv_name);
         }

         this->pd->pending_put_callback = false;          // clear
         this->pd->connectionStatus = PrivateData::csDisconnected;

         // We unsubscribe here to avoid a duplicate subscriptions if/when we
         // re-connect. In principle we could keep the same subscription active,
         // but doing a new Subscribe on connect will do a new Array_Get and new
         // Subscribe which is good in case any PV meta data parameters (units,
         // precision, num elements, and even native type) have changed.
         //
         this->unsubscribeChannel ();

         // Any buffered data is now meaningless.
         //
         this->pd->clearBuffer ();
         this->callConnectionUpdate ();
         break;

      default:
         reportError ("connection_handler: Unexpected args op");
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client::eventHandler (struct event_handler_args& args)
{
   // Valid channel id - need some more event specific checks.
   //
   // Treat Get and Sub(scription) events the same.
   //
   if ((args.usr == &Get) || (args.usr == &Sub)) {

      if (args.status == ECA_NORMAL) {

         if (args.dbr) {
            this->updateHandler (args);
         } else {
            reportError ("event_handler (%s) args.dbr is null",
                         this->pd->pv_name);
         }

      } else {
         reportError ("event_handler Get/Sub (%s) error (%s)",
                       this->pd->pv_name, ca_message (args.status));
      }

   } else if (args.usr == &Put) {
      if (this->pd->pending_put_callback) {
         // Clear pending flag.
         //
         this->pd->pending_put_callback = false;
         this->callPutCallbackNotifcation (args.status == ECA_NORMAL);

      } else {
         reportError ("event_handler (%s) unexpected put call back",
                      this->pd->pv_name);
      }

   } else {
      reportError ("event_handler (%s) unexpected args.usr",
                   this->pd->pv_name);
   }
}


//------------------------------------------------------------------------------
//
void ACAI::Client::setConnectionHandler (ConnectionHandlers eventHandler)
{
   this->connectionUpdateEventHandler = eventHandler;
}

//------------------------------------------------------------------------------
//
ACAI::Client::ConnectionHandlers ACAI::Client::connectionHandler () const
{
   return this->connectionUpdateEventHandler;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setUpdateHandler (UpdateHandlers eventHandler)
{
   this->dataUpdateEventHandler = eventHandler;
}

//------------------------------------------------------------------------------
//
ACAI::Client::UpdateHandlers ACAI::Client::updateHandler () const
{
   return this->dataUpdateEventHandler;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::setPutCallbackHandler (PutCallbackHandlers putCallbackHandler)
{
   this->putCallbackEventHandler = putCallbackHandler;
}

//------------------------------------------------------------------------------
//
ACAI::Client::PutCallbackHandlers ACAI::Client::putCallbackHandler () const
{
   return this->putCallbackEventHandler;
}

//------------------------------------------------------------------------------
//
void ACAI::Client::connectionUpdate (const bool)
{
   // place holder - not quite a pure abstract function
}

//------------------------------------------------------------------------------
//
void ACAI::Client::dataUpdate (const bool)
{
   // place holder - not quite a pure abstract function
}

//------------------------------------------------------------------------------
//
void ACAI::Client::putCallbackNotifcation (const bool)
{
   // place holder - not quite a pure abstract function
}


//------------------------------------------------------------------------------
//
void ACAI::Client::callConnectionUpdate ()
{
   bool isConnected;

   // Create a pseudo update time.
   //
   this->pd->timeStamp = epicsTime::getCurrent ();

   // Call hook function, but only if necessary, i.e. if status has changed.
   //
   isConnected = this->isConnected ();
   if (this->pd->lastIsConnected != isConnected) {
      this->pd->lastIsConnected = isConnected;

      // This is a dispatching call. This is done FIRST in order to ensure that
      // the object performs any necessary self updates prior to notifiy other
      // users of the change.
      //
      this->connectionUpdate (isConnected);

      // Call registered users.
      //
      ITERATE (RegisteredUsers, this->registeredUsers, user) {
         (*user)->connectionUpdate (this, isConnected);
      }

      // Call event handler.
      //
      if (this->connectionUpdateEventHandler) {
         this->connectionUpdateEventHandler (this, isConnected);
      }
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client::callDataUpdate (const bool isFirstUpdateIn)
{
   // First update: This is done FIRST in order to ensure that the object performs
   // any self necessary updates prior to notifiying other users of the change.
   //
   // Call hook function - this is a dispatching call.
   //
   this->dataUpdate (isFirstUpdateIn);

   // Second update: Call registered users.
   //
   ITERATE (RegisteredUsers, this->registeredUsers, user) {
      (*user)->dataUpdate (this, isFirstUpdateIn);
   }

   // Third update: Call event handler.
   //
   if (this->dataUpdateEventHandler) {
      this->dataUpdateEventHandler (this, isFirstUpdateIn);
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client::callPutCallbackNotifcation (const bool isSuccessfulIn)
{
   // First update: This is done FIRST in order to ensure that the object performs
   // any self necessary updates prior to notifiying other users of the change.
   //
   // Call hook function - this is a dispatching call.
   //
   this->putCallbackNotifcation (isSuccessfulIn);

   // Second update: Call registered users.
   //
   ITERATE (RegisteredUsers, this->registeredUsers, user) {
      (*user)->putCallbackNotifcation (this, isSuccessfulIn);
   }

   // Third update: Call event handler.
   //
   if (this->putCallbackEventHandler) {
      this->putCallbackEventHandler (this, isSuccessfulIn);
   }
}


// TODO: Must/should mutex access to this ??
static struct ca_client_context*  acai_context = NULL;

//------------------------------------------------------------------------------
// static
bool ACAI::Client::initialise ()
{
   int status;

   // Perform sanity check.
   //
   if (sizeof (ACAI::ClientInteger) != sizeof (epicsInt32)) {
      reportError ("Size of ACAI::ClientInteger is incompatible with epicsInt32");
      return false;
   }

   if (sizeof (ACAI::ClientFloating) != sizeof (epicsFloat64)) {
      reportError ("Size of ACAI::ClientFloating is incompatible with epicsFloat64");
      return false;
   }

   initialise_buffered_callbacks ();

   // Create Channel Access context.
   //
   status = ca_context_create (ca_enable_preemptive_callback);
   if (status != ECA_NORMAL) {
      reportError ("ca_context_create failed - %s", ca_message (status));
      return false;
   }

   acai_context = ca_current_context ();  // save current context

   // Replace the CA Library report handler.
   //
   status = ca_replace_printf_handler (buffered_printf_handler);
   if (status != ECA_NORMAL) {
      reportError ("ca_replace_printf_handler failed - %s",
                   ca_message (status));
      //
      // This is not exit-worthy. Carry on
   }

   return true;
}

//------------------------------------------------------------------------------
// static
bool ACAI::Client::attach ()
{
   bool result = false;
   if (acai_context) {
      int status = ca_attach_context (acai_context);
      result = (status == ECA_NORMAL);
      if (! result) reportError ("ca_attach_context failed - %s", ca_message (status));
   } else {
      reportError ("attach failed - there is no current acai context: call ACAI::Client::initialise ()");
   }
   return result;
}

//------------------------------------------------------------------------------
// static
void ACAI::Client::finalise ()
{
   // Reset the CA Library report handler - no error check.
   //
   ca_replace_printf_handler (NULL);
   ca_context_destroy ();
   acai_context = NULL;

   clear_all_buffered_callbacks ();
}

//------------------------------------------------------------------------------
// static
void ACAI::Client::poll (const int maximum)
{
   // The acai_context variable signifies if initialise has been called.
   // If it has not been called then do nothing.
   //
   if (!acai_context) return;

   const int status = ca_flush_io ();
   if (status != ECA_NORMAL) {
      reportError ("ca_flush_io failed - %s", ca_message (status));
   }

   process_buffered_callbacks (maximum);
}


//==============================================================================
// Meta data get functions - boiler plate stuff - returns a semi-sensible
// value if the channel is not connected.
//
#define GET_META_DATA(type, getName, member, when_not_connected)     \
                                                                     \
type ACAI::Client::getName () const                                  \
{                                                                    \
   if (this->isConnected ()) {                                       \
      return (type) this->pd->member;                                \
   } else {                                                          \
      return (type) when_not_connected;                              \
   }                                                                 \
}


//             type                        getName             member                 when_not_connected
GET_META_DATA (ACAI::ClientAlarmSeverity,  alarmSeverity,      severity,              ClientDisconnected)
GET_META_DATA (ACAI::ClientAlarmCondition, alarmStatus,        status,                   ClientAlarmNone)
GET_META_DATA (int,                        precision,          precision,                              0)
GET_META_DATA (ACAI::ClientString,         units,              units,                                 "")
GET_META_DATA (double,                     lowerDisplayLimit,  lower_disp_limit,                     0.0)
GET_META_DATA (double,                     upperDisplayLimit,  upper_disp_limit,                     0.0)
GET_META_DATA (double,                     lowerControlLimit,  lower_ctrl_limit,                     0.0)
GET_META_DATA (double,                     upperControlLimit,  upper_ctrl_limit,                     0.0)
GET_META_DATA (double,                     lowerWarningLimit,  lower_warning_limit,                  0.0)
GET_META_DATA (double,                     upperWarningLimit,  upper_warning_limit,                  0.0)
GET_META_DATA (double,                     lowerAlarmLimit,    lower_alarm_limit,                    0.0)
GET_META_DATA (double,                     upperAlarmLimit,    upper_alarm_limit,                    0.0)
GET_META_DATA (ACAI::ClientString,         hostName,           channel_host_name,                     "")
GET_META_DATA (unsigned int,               hostElementCount,   channel_element_count,                  0)
GET_META_DATA (ACAI::ClientFieldType,      hostFieldType,      host_field_type,     ClientFieldNO_ACCESS)
GET_META_DATA (ACAI::ClientFieldType,      dataFieldType,      data_field_type,     ClientFieldNO_ACCESS)
GET_META_DATA (unsigned int,               dataElementCount,   data_element_count,                     0)

#undef GET_META_DATA

//------------------------------------------------------------------------------
//
unsigned int ACAI::Client::dataElementSize () const
{
   if (this->dataIsAvailable ()) {
      return this->pd->data_field_size;
   } else {
      return 0;
   }
}

//------------------------------------------------------------------------------
// static
const char* ACAI::Client::dbRequestTypeImage (const long type)
{
   const char* result = "";
   if (dbr_type_is_valid (type)) {
      result = dbr_text [type];
   } else {
      result = dbf_text_invalid;
   }
   return result;
}

//------------------------------------------------------------------------------
// static
ACAI::Client* ACAI::Client::validateChannelId (const void* channel_idx)
{
   const chid channel_id = (const chid) channel_idx;

   void* user_data = NULL;
   ACAI::Client* result = NULL;

   // Hypothesize something wrong unless we pass all checks.
   //
   if (channel_id == NULL) {
      reportError ("Unassigned channel id");
      return NULL;
   }

   user_data = ca_puser (channel_id);
   if (user_data == NULL) {
      reportError ("validateChannelId: Unassigned channel_id user data");
      return NULL;
   }

   result = (Client *) user_data;
   if (result->magic_number != MAGIC_NUMBER_C) {
      // Although a sensible check, no need to report this - it is not
      // unexpected to get an update for a channel that has just recently
      // been deleted (magic_number fields are cleared when client deleted).
      //
      // reportError ("User data does not reference a ACAI::Client object (magic number check)");
      //
      return NULL;
   }

   if (result->pd == NULL) {
      reportError ("validateChannelId: client has no associated private data");
      return NULL;
   }

   if (result->pd->magic_number != MAGIC_NUMBER_P)  {
      // Although a sensible check, no need to report this - it is not
      // unexpected to get an update for a channel that has just recently
      // been deleted.
      //
      return NULL;
   }

   if (result->pd->channel_id == NULL) {
      // Although a sensible check, no need to report this - it is not
      // unexpected to get an update for a channel that has just recently
      // been closed.
      //
      return NULL;
   }

   if (result->pd->channel_id != channel_id) {
      // Similarly this a sensible check and also no need to report this error,
      // it is not unexpected for this to occur if/when a client is recycled.
      //
      return NULL;
   }

   // We passed all the checks ;-)
   //
   return result;

}  // end Validate_Channel_Id


//------------------------------------------------------------------------------
//
void ACAI::Client::registerUser (ACAI::Abstract_Client_User* user)
{
   if (!user) return;  // sanity check

   this->registeredUsers.insert (user);
}

//------------------------------------------------------------------------------
//
void ACAI::Client::deregisterUser (ACAI::Abstract_Client_User* user)
{
   if (!user) return;  // sanity check

   this->registeredUsers.erase (user);
}

//------------------------------------------------------------------------------
//
void ACAI::Client::removeClientFromAllUserLists ()
{
   // Client about to be deleted - remove from any interested user lists.
   //
   ITERATE (RegisteredUsers, this->registeredUsers, userRef) {
      ACAI::Abstract_Client_User* user = *userRef;
      if (user) {
         user->removeClientFromList (this);
      }
   }
   this->registeredUsers.clear ();
}


//==============================================================================
// Channel Access library call back handling
//
// ACAI::Client_Private is a friend class of ACAI::Client, and as such is allowed
// access to private ACAI::Client class functions, specifically:
//
//    validateChannelId ()
//
// This is why this functionality is implemented as a class as opposed to just a
// couple of static functions.
//
namespace ACAI {

class Client_Private {
public:

   //---------------------------------------------------------------------------
   //
   static void connectionHandler (struct connection_handler_args& args)
   {
      ACAI::Client *pClient;

      pClient = ACAI::Client::validateChannelId (args.chid);
      if (pClient) {
         pClient->connectionHandler (args);
      }
   }

   //---------------------------------------------------------------------------
   //
   static void eventHandler (struct event_handler_args& args)
   {
      ACAI::Client *pClient;

      pClient = ACAI::Client::validateChannelId (args.chid);
      if (pClient) {
         pClient->eventHandler (args);
      }
   }
};

}

//------------------------------------------------------------------------------
// Buffered callbacks is a C module, not C++.
// These functions intended to be only called by the buffered callback module.
// Note they pass back pointer parameters.
//
extern "C" {
void application_connection_handler (struct connection_handler_args* args);
void application_event_handler (struct event_handler_args* args);
void application_printf_handler (char* formated_text);
}

//------------------------------------------------------------------------------
// Connection handler
//
void application_connection_handler (struct connection_handler_args* args)
{
   // Apply sainity check and deferance.
   //
   if (args) ACAI::Client_Private::connectionHandler (*args);
}

//------------------------------------------------------------------------------
// Event handler
//
void application_event_handler (struct event_handler_args* args)
{
   // Apply sainity check and deferance.
   //
   if (args) ACAI::Client_Private::eventHandler (*args);
}

//------------------------------------------------------------------------------
// Replacement printf handler
//
void application_printf_handler (char* formated_text)
{
   // Note strings already end with '\n'.
   //
   fprintf (stderr, "%s", formated_text);
}

// end
