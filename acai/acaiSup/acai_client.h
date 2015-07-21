/* $File: //depot/sw/epics/acai/acaiSup/acai_client.h $
 * $Revision: #11 $
 * $DateTime: 2015/06/22 20:47:28 $
 * $Author: andrew $
 *
 * This file is part of the ACAI library. 
 * The class derived from pv_client developed for the kryten application.
 *
 * Copyright (C) 2014,2015  Andrew C. Starritt
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

#ifndef ACAI__CLIENT_H_
#define ACAI__CLIENT_H_

#include <set>
#include <acai_client_types.h>

namespace ACAI {

class Abstract_Client_User;    // differed declaration.

/// \brief The ACAI::Client class is main class within the ACAI library.
///
/// While the ACAI::Client_Set and ACAI::Abstract_Client_User classes provide
/// some usefull meta functionality, an application can be created that only uses
/// ACAI::Client objects.
///
/// An ACAI::Client object has number of attributes, but of primary importance is it's
/// process variable (PV) name. The default settings for other attributes often suitable
/// and as such need not be changed.
/// The PV name attribute is the only attribute that may be set during construction,
/// all other attributes have their own set and get functions.
///
/// It should be noted that many attributes only take effect when Client::openChannel
/// is called (which calls ca_create_channel) or when a successfull connection occurs
/// (when ca_array_get_callback and ca_create_subscription are called).
///
/// When a channel disconnects any subscription is cancelled. If/when the channel
/// reconnects, the channels meta data is re-read and the subscription reestablished.
/// Rationale: the underlying meta data may haved changed (precision, engineering units,
/// enumeration state values etc.), or even the field type and/or number of elements
/// may have changed.
///
/// PV data get and put functions.
/// The get functions extract data currently stored in the object from the most recent update.
/// The put functions calls the underlying ca_put_array function. Put data goes "around
/// the loop", i.e. to the IOC and back again as an event update before the object's data
/// is updated and available to the get functions.
///
/// ACAI Client users may be notified of connection and/or update events in one
/// of three ways. These are (and occur in the order listed below:
///
/// a) Override the class Client::connectionUpdate and Client::dataUpdate virtual functions
///    in a derived class;
///
/// b) Derive a class from ACAI::Abstract_Client_User, override the class
///    Abstract_Client_User::connectionUpdate and Abstract_Client_User::dataUpdate
///    functions and register clients of interest; and/or
///
/// c) Invoke Client::setConnectionHandler and Client::setUpdateHandler
///    to definea traditional callback handlers function (only one connection
///    and one event handler per client);
///
/// It is intended that appications may use the ACAI::Client class instances directly
/// or derive their own client classes using ACAI::Client as a base class. A number
/// of functions have been declared virtual specifically to support this.
/// The virtual functions are Client::~Client, Client::getString, Client::connectionUpdate
/// and Client::dataUpdate
///
class ACAI_SHARED_CLASS Client {
public:
   // class types --------------------------------------------------------------
   //
   // While C/C++ treats this types as essentially the same, they should be
   // considered as different and distinct types.
   //
   /// Defines the traditional connection handler function signature.
   /// This call back is set by Client::setConnectionHandler and read by Client::connectionHandler.
   ///
   typedef void (*ConnectionHandlers) (ACAI::Client* client, const bool isConnected);

   /// Defines the traditional event/data update handler function signature.
   /// This call back is set by Client::setUpdateHandler and read by  Client::updateHandler.
   ///
   typedef void (*UpdateHandlers)     (ACAI::Client* client, const bool firstUpdate);


   // static functions ---------------------------------------------------------
   //
   // Thes fuctiions handle context creation/deletion and calls to flush io.
   //
   /// The initialise function must be the first ACAI fuction called.
   /// It should be called in the thread that is to be used for channel access.
   /// Note: this class does not address multiple contexts.
   //
   static void initialise ();

   /// The finalise should be called if/when the ACAI libray fuctionality is no longer required.
   /// This includes when the application terminates and the application wishes to "play nice".
   ///
   static void finalise ();

   /// Call on regular basis, say every 10-50 mSec.
   /// Under the covers, this fuction calls ca_flush_io and then processes any buffered callbacks.
   /// The maximum parameter specifies max number of items buffered processed.
   ///
   /// NOTE: It is this function that goes on to call the virtual Client::connectionUpdate
   /// and Client::dataUpdate function, the Abstract_Client_User::connectionUpdate
   /// and Abstract_Client_User::dataUpdate functions for registered clients as
   /// well as any Client::ConnectionHandlers and Client::UpdateHandlers callback functions.
   ///
   static void poll (const int maximum = 800);


   // object functions ---------------------------------------------------------
   //
   /// Class constructor. The PV name may be set during construction or (re)set
   /// later on using Client::setPvName. All other attributes have theitr own set
   /// and get functions.
   ///
   explicit Client (const ClientString& pvName = "");

   /// Class destructor.
   ///
   virtual ~Client ();

   /// Sets or resets the channel PV name.
   /// Unless doImmediateReopen is set true, setting PV name while channel connected
   /// has no immediate effect. The channel must be closed and re-opended.
   /// When doImmediateReopen is true the function closes and reopnes the channel
   /// immediately.
   ///
   void setPvName (const ACAI::ClientString& pvName, const bool doImmediateReopen = false);

   /// Returns the current channel name as a ClientString.
   ///
   ClientString pvName () const;

   /// Returns the current channel name as a traditional c-style string.
   ///
   const char* cPvName () const;

   /// This function sets the data request type to one of the stannndard EPICS field types.
   ///
   /// By default, the date data request type is ClientFieldDefault and as such the data
   /// subscription request is based on the host (IOC) native field type, e.g. if
   /// the native field type if DBF_DOUBLE (ClientFieldDOUBLE) then the request is
   /// DBF_DOUBLE (ClientFieldDOUBLE). However, a client user may choose to override
   /// this by setting the the data request type.
   ///
   /// Note: once the channel is open, updating the request field type has no effect
   /// on the the current subscription updates. The request field type is essentially
   /// only used when the channel first connects or re-connects.
   ///
   void setDataRequestType (const ACAI::ClientFieldType fieldType);

   /// Returns the current data request type.
   ///
   ACAI::ClientFieldType  dataRequestType () const;

   /// This function limits the number of elements requested from the server.
   /// If this limit is not specified then all PV elements are requested.
   ///
   /// Note: In either case (limit or no limit), the actual number of PV elements
   /// requested is subject to any constraint imposed by the EPICS_CA_MAX_ARRAY_BYTES
   /// environment variable and the number of elements availbe from the server
   /// (as indicated by the hostElementCount function).
   //
   /// Note: for newer versions of EPICS, IOCs interpret a request for zero elements
   /// as the number of defined elements (e.g. NORD field value in a waveform record).
   ///
   void setRequestCount (const unsigned int number);

   /// This function clears any number of elements requested limits set by setRequestCount.
   ///
   void clearRequestCount ();

   /// Returns the current number of elements requested limit.
   /// The returned value is only applicable when isDefined is true.
   ///
   unsigned int requestCount (bool& isDefined) const;

   /// Set channel priorty. Limited to 0 .. 99, defaults to 10.
   /// The default is greater than 0 so that lesser values may be
   /// specified for large (e.g. image) array PVs.
   ///
   /// Note: The channle priority only taked effect the next time the channel is opened.
   ///
   void setPriority (const unsigned int priority);

   /// Returns the current client priority.
   ///
   unsigned int priority () const;

   /// When set, arrays of DBF_CHAR interpretted as string by getString/putString.
   /// Note: Since base 3.14.11, this is implicit for certain field types when '$'
   /// appended to the field name.
   //
   void setLongString (const bool isLongString);

   /// Returns the current long string setting.
   ///
   bool isLongString () const;

   /// Sets whether the channel is opened in subscrbing mode or single read mode or no read mode.
   /// When readMode is Subscribe, the client subscribes for updates.
   /// When readMode is SingleRead, the client does a single read per connection.
   /// The default setting when the client object is constructed is Subscribe.
   ///
   /// Note: once the channel is open, updating the readMode hase no effect on the
   /// the current subscription (or lack there of). The readMode is essentially
   /// only used when the channel first connects or re-connects.
   ///
   void setReadMode (const ACAI::ReadModes readMode);

   /// Returns the current client read mode.
   ///
   ACAI::ReadModes readMode () const;

   /// Create channel, and once connected and read data (with all meta data) and
   /// optionally subscribe for updates. Returns true if create channel okay.
   ///
   bool openChannel ();

   /// Clear any subscription and close associated channel.
   ///
   void closeChannel ();

   /// Conveniance function to call closeChannel and then openChannel.
   ///
   bool reopenChannel ();

   /// When a channel is connected, this function causes the data to be re-read once.
   /// If not connected this function does nothing.
   /// This intented for channels opened with read mode set to SingleRead (or NoRead)
   /// however this will force a reread (including meta data) of a subscribing object.
   ///
   bool reReadChannel ();

   /// Returns whether the channel is currenlty connected. The put data functions may
   /// be used on a connected channel, however the dataIsAvailable function should be
   /// used prior ti called the get data functions.
   ///
   bool isConnected () const;

   /// Returns whether channel data is currently avilable, accessable via the get
   /// data functions. Data available implies the channel is connected, but connected
   /// does not imply data available.
   //
   bool dataIsAvailable () const;

   // If the channel is closed, these functions return default values (0, 0.0,
   // "", etc. depending upon the data type). If user cares enough then
   // dataIsAvailable should be used in order to determine the veracity of
   // the returned values.
   //
   /// For a connected channel, this fuction returns the precision typically
   /// specified in a record's PREC field. When the channel is not connected,
   /// the function returns 0.
   ///
   int precision () const;

   /// For a connected channel, this fuction returns the associated engineering
   /// units typically specified in a record's EGU field. When the channel is not
   /// connected, the function returns "".
   ///
   ClientString units () const;

   /// For a connected channel, this fuction returns the channel's lower operating
   /// range typically specified in a record's LOPR field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double lowerDisplayLimit () const;
   
   /// For a connected channel, this fuction returns the channel's upper operating
   /// range typically specified in a record's HOPR field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double upperDisplayLimit () const;

   /// For a connected channel, this fuction returns the channel's lower control
   /// limit typically specified in a record's DRVL field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double lowerControlLimit () const;

   /// For a connected channel, this fuction returns the channel's upper control
   /// limit typically specified in a record's DRVH field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double upperControlLimit () const;

   /// For a connected channel, this fuction returns the channel's lower warning
   /// limit typically specified in a record's LOW field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double lowerWarningLimit () const;

   /// For a connected channel, this fuction returns the channel's upper warning
   /// limit typically specified in a record's HIGH field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double upperWarningLimit () const;

   /// For a connected channel, this fuction returns the channel's lower alarm
   /// limit typically specified in a record's LOLO field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double lowerAlarmLimit () const;

   /// For a connected channel, this fuction returns the channel's upper alarm
   /// limit typically specified in a record's HIHI field. When the channel is
   /// not connected, the function returns 0.0
   ///
   double upperAlarmLimit () const;

   /// For a connected channel, this fuction returns the channel's hostname or IP address.
   /// When the channel is not connected, the function returns "".
   ///
   /// Note: this will be the EPICS gateway's hostname or IP address as opposed to
   /// the IOC's hostname or IP address.
   ///
   ClientString hostName () const;

   /// Returns the nuber of PV array elements as defined by PV server (IOC).
   ///
   unsigned int hostElementCount () const;

   /// Returns the nuber of PV array elements available in the clinet objects,
   /// i.e.  as returned by most recent update.
   /// 
   /// This can be less than hostElementCount because it has been limited by a
   /// call to setRequestCount and/or because the EPICS_CA_MAX_ARRAY_BYTES
   /// environment variables is less than is needed to handle all elements.
   ///  
   unsigned int dataElementCount () const;

   /// Returns the PV's native field type as defined by PV server (IOC).
   /// 
   ClientFieldType hostFieldType () const;

   /// Returns the data type stored within the client object as returned by
   /// most recent update - see setDataRequestType.
   ///
   ClientFieldType dataFieldType () const;

   /// Returns the data element, e.g. DBF_LONG = 4
   ///
   unsigned int dataElementSize () const;

   /// Returns the channel channel alarm status.
   ///
   ClientAlarmCondition alarmStatus () const;
   
   /// Returns the channel channel alarm severity. This maps directly to the regular
   /// channel severity as provided by Channel Access, but for a disconnected channel
   /// this function returns ACAI::ClientDisconnected which is "more sever" than
   /// ACAI::ClientSevInvalid.
   ///
   ClientAlarmSeverity alarmSeverity () const;

   /// This function returns a textual/displayable form of the channel's severity.
   ///
   ClientString alarmStatusImage () const;      // alarmStatusString defined as macro

   /// This function returns a textual/displayable form of the channel's alarm status.
   ///
   ClientString alarmSeverityImage () const;    // alarmSeverityString defined as macro

   /// This function returns the channel's read access permission.
   ///
   bool readAccess () const;

   /// This function returns the channel's write access permission.
   ///
   bool writeAccess () const;

   /// Get PV last update time (or connect/disconnect time). For updates, this is
   /// the time embedded within the CA data, however for connection/disconnections
   /// this is the time as determined from the clint's host.
   ///
   time_t utcTime (int * nanoSecOut = NULL) const;

   /// Get the time stamp of the most recent connection/data update event.
   /// Data update time stamps are based on the time stamp provided by the EPICS
   /// PV server, while connection/disconnection time stamps are based on local
   /// time.
   ClientTimeStamp timeStamp () const;

   /// Format is: "yyyy-mm-dd hh:nn:ss[.ffff]"
   /// Without fractions, this is a suitable format for MySql.
   ///
   ClientString utcTimeImage (int precision = 0) const;

   // Get PV value as basic scaler. For array (e.g. waveform) records, index
   // can be used to specify which element of the array is required.
   // Array elements are indexed from zero - this is C++ after all.
   //
   /// Returns the index-th element of the channle data store in the client as a floating value.
   ///
   ClientFloating getFloating (unsigned int index = 0) const;

   /// Returns the index-th element of the channle data store in the client as an integer value.
   ///
   ClientInteger  getInteger  (unsigned int index = 0) const;

   /// Returns the index-th element of the channle data store in the client as a string value.
   ///
   /// Performs basic string formatting for numeric and enumeration types.
   /// Includes units if specified by setIncludeUnits (true).
   /// More elaborate string formatting for numeric types is beyond the scope
   /// of this class and is best handled at the application level.
   ///
   /// NOTE: This function is virtual and may be overridden by a sub-class in
   /// order to allow a more sophisticated formatting if required.
   ///
   virtual ClientString getString (unsigned int index = 0) const;

   /// Get client array (waveform) data as an array of floating values.
   ///
   ClientFloatingArray getFloatingArray () const;

   /// Get client array (waveform) data as an array of integer values.
   ///
   ClientIntegerArray  getIntegerArray  () const;

   /// Get client array (waveform) data as an array of string values.
   ///
   ClientStringArray   getStringArray   () const;

   // MAYBE: Add put_callback on/off switch or optional parameter??
   /// Write scaler value to channel.
   /// On the wire (via CA protocol) we use DBF_DOUBLE format, not the PV's native field format.
   ///
   bool putFloating (const ACAI::ClientFloating value);

   /// Write scaler value to channel.
   /// On the wire (via CA protocol) we use DBF_LONG format, not the PV's native field format.
   ///
   bool putInteger  (const ACAI::ClientInteger  value);

   /// Write scaler value to channel.
   /// On the wire (via CA protocol) we use DBF_STRING format, not the PV's native field format.
   /// The string is truncated if necessary.
   ///
   bool putString   (const ACAI::ClientString&  value);   // tuncated if needs be.

   // puts using std::vector types.
   //
   /// Write a floating vector array value to the channel.
   bool putFloatingArray (const ACAI::ClientFloatingArray& valueArray);

   /// Write a integer vector array value to the channel.
   bool putIntegerArray  (const ACAI::ClientIntegerArray&  valueArray);

   /// Write a string vector array value to the channel.
   bool putStringArray   (const ACAI::ClientStringArray&   valueArray);

   // puts using classic arrays.
   //
   /// Write a traditional floating array value to the channel.
   bool putFloatingArray (const ACAI::ClientFloating* valueArray, const unsigned int count);

   /// Write a traditional integer array value to the channel.
   bool putIntegerArray  (const ACAI::ClientInteger*  valueArray, const unsigned int count);

   /// Write a traditional string array value to the channel.
   bool putStringArray   (const ACAI::ClientString*   valueArray, const unsigned int count);

   /// Extract the channel enumeration state strings if they exist.
   /// This function gets the enum state string, not the state string for the
   /// n-th element of a waveform of enums.
   ///
   /// Use Client::getInteger/Client::getIntegerArray to get enum values.
   ///
   /// NOTE: Does a special for {recordname}.STAT which has more than 16 states.
   /// Others (like sscan.FAZE) are not addressed here.
   ///
   ClientString getEnumeration (int state) const;

   /// Get all state strings.
   ///
   ClientStringArray getEnumerationStates () const;

   /// Extract the number of enumeration states.
   ///
   int enumerationStatesCount () const;

   /// Raw data access - mainly intended for large waveforms.
   ///
   /// Return size of raw data in bytes - excludes meta data.
   ///
   size_t rawDataSize () const;

   /// Extracts count bytes from the channel access data into dest. If less than
   /// count bytes available, then only available bytes are extracted. The actual
   /// number of bytes extracted is returned via the count parameter.
   ///
   void rawDataPointer (void* dest, const size_t size, unsigned int& count) const;

   /// Returns a pointer to the actual data. The actual number of bytes available
   /// is returned in count.
   //
   /// NOTE: The data pointer returned is only guarenteed valid until the next call
   /// to Client::poll ().
   /// Do NOT store pointer - only valid during dataUpdate function call.
   /// Do NOT write to the data - use as read only access.
   ///
   const void* rawDataPointer (unsigned int& count) const;

   /// This function modify sets the channel's include units attrbute.
   /// The default attribute value when the client object is constructed is false.
   /// This attrbute moifies the getString and getStringArray functionality.
   //
   void setIncludeUnits (const bool includeUnitsIn);

   /// Returns the client's include units attrbute.
   ///
   bool includeUnits () const;

   /// Set traditional callback handler, which is called in response
   /// to a connection event.
   ///
   /// Only one per client can be set at any one time.
   /// Note: Last in - best dressed!
   ///
   /// Also see Abstract_Client_User class and the connectionUpdate function
   /// for alternative notification methods.
   //
   void setConnectionHandler (ConnectionHandlers eventHandler);

   /// Returns the client's connection handler function reference.
   ///
   ConnectionHandlers connectionHandler () const;

   /// Set traditional callback handler, which is called in response
   /// to a data update event .
   ///
   /// Only one per client can be set at any one time.
   /// Note: Last in - best dressed!
   ///
   /// See Client::updateHandler.
   ///
   /// Also see Client::dataUpdate and Abstract_Client_User::dataUpdate functions
   /// for alternative notification methods.
   //
   void setUpdateHandler (UpdateHandlers updateHandler);

   /// Returns the client's update handler function reference.
   ///
   UpdateHandlers updateHandler () const;

   /// An int tag: not used by the class per se but available for access by client
   /// users and call back handlers to use and abuse as they see fit.
   ///
   int userTag;

   /// A reference tag: not used by the class per se but available for access by client
   /// users and call back handlers to use and abuse as they see fit.
   ///
   void* userRefTag;

   /// A string tag: not used by the class per se but available for access by client
   /// users and call back handlers to use and abuse as they see fit.
   ///
   ClientString userStringTag;

protected:
   /// This is a hook functions. It should not / can not be called from
   /// outside of the ACAI::Client, but may be overriden by sub classes to
   /// allow them to handle connections events.
   /// Note: This function is called prior to any connection callback handler.
   ///
   virtual void connectionUpdate (const bool isConnected);

   /// This is a hook functions. It should not / can not be called from
   /// outside of the ACAI::Client, but may be overriden by sub classes to
   /// allow them to handle event updates.
   /// Note: This function is called prior to any event callback handlers.
   ///
   virtual void dataUpdate (const bool firstUpdate);

   /// Determines if this is a record's status PV.
   /// Essentialy checks if the PV name ends with ".STAT"
   ///
   bool isAlarmStatusPv () const;

   /// Determines if long string processing required. This may be either
   /// explicit (by setting longString to true - field type must be DBF_CHAR)
   /// or implicit (PV name ends with $).
   //
   /// Note: As of 3.14.11, IOCs now provides support for strings longer than 40
   /// characters. Adding the suffix '$' to the field name of an IOC PV name
   /// causes the native type of that field to be reported as DBF_CHAR and number
   /// of element sufficient to cope with actual field length provided that the
   /// actual field type is one of:
   /// DBF_STRING, DBF_INLINK, DBF_OUTLINK or DBF_FWDLINK.
   ///
   bool doProcessAsLongString () const;

private:
   int magic_number;    // used to verify void* to Client* conversions.

   // Allows the private data to be truely private, which in turn means
   // that we need not include EPICS header files in this header file.
   //
   class PrivateData;
   PrivateData* pd;

   // Traditional callback handlers.
   //
   ConnectionHandlers connectionUpdateEventHandler;
   UpdateHandlers dataUpdateEventHandler;

   // Registered users.
   //
   typedef std::set<Abstract_Client_User*> RegisteredUsers;
   RegisteredUsers registeredUsers;

   // These function are used within the ACAI::Client object to
   // call the corresponsing hook functions, registered abstract user hook
   // functions and call back event handlers.
   //
   void callConnectionUpdate ();
   void callDataUpdate (const bool firstUpdateIn);

   /// Manage Client/Abstract_Client_User associations.
   ///
   void registerUser (Abstract_Client_User* user);
   void deregisterUser (Abstract_Client_User* user);
   void removeClientFromAllUserLists ();

   friend class Abstract_Client_User;

   bool readChannel (const ACAI::ReadModes readMode);
   void unsubscribeChannel ();

   void connectionHandler (struct connection_handler_args* args);
   void updateHandler (struct event_handler_args* args);
   void eventHandler (struct event_handler_args* args);

   friend class Client_Private;

   // Validates the channel id. If valid, returns referance to a ACAI::Client
   // object otherwise returns NULL.
   //
   static Client* validateChannelId (const void* channel_id);
};

}

#endif   // ACAI__CLIENT_H_