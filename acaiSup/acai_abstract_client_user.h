/* acai_abstract_client_user.h
 *
 * This file is part of the ACAI library. It provides a base class that
 * supports application classes that use this library.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 */

#ifndef ACAI_ABSTRACT_CLIENT_USER_H_
#define ACAI_ABSTRACT_CLIENT_USER_H_

#include "acai_client_types.h"
#include "acai_client_set.h"
#include "acai_shared.h"

namespace ACAI {

class Client;       // differed declaration.

/// \brief The ACAI::Abstract_Client_User is a base class provided to support
/// application classes that use the ACAI library.
///
/// An instance of a class derived from the ACAI::Abstract_Client_User class may
/// register zero, one or more ACAI::Client or derived class objects.
/// This means that when one of those registered objects connects/diconnects or
/// receives an event update, it will invoke the connectionUpdate, dataUpdate or
/// putCallbackNotifcation virtual function of this object.
///
/// It is intended that appications derive their own classes using this class as a
/// base class. A number of functions have been declared virtual specifically to
/// support this. The virtual functions are:
///    Abstract_Client_User::~Abstract_Client_User,
///    Abstract_Client_User::connectionUpdate,
///    Abstract_Client_User::dataUpdate, and
///    Abstract_Client_User::putCallbackNotifcation
///
/// The ACAI::Abstract_Client_User to ACAI::Client association can be a many-to-many,
/// although this would typically be one-to-many. While a one-to-none mapping is not
/// prohibited, this would be next to useless.
///
/// Note: the corresponding ACAI::Client register/deregister functions are private.
/// The association can only be managed through the ACAI::Abstract_Client_User class
/// API. This is because the underlying implementation is asymmetric.
///
class ACAI_SHARED_CLASS Abstract_Client_User  {
public:
   /// Constructor - initialises the object.
   ///
   explicit Abstract_Client_User ();

   /// NOTE: All clients are automatically degreistered when this object is deleted.
   ///
   virtual ~Abstract_Client_User ();

   /// Register this user for conection/update events associated with the nominated
   /// client.
   ///
   void registerClient (ACAI::Client* client);

   /// Register this user for conection/update events associated with the specified
   /// set of clients.
   ///
   void registerAllClients (ACAI::Client_Set* clientSet);

   /// Deregister this user with the nominated client.
   ///
   /// NOTE: Any client is automatically degreistered if/when the client object
   /// is deleted.
   ///
   void deregisterClient (ACAI::Client* client);

   /// Deregister this user for conection/update events associated with the specified
   /// set of clients.
   ///
   void deregisterAllClients (ACAI::Client_Set* clientSet);

   /// Tests whether the specified client is registered.
   ///
   bool clientIsRegistered (ACAI::Client* client) const;

   /// This function opens all currently registered channels.
   /// The openRegisteredChannels returns true if all channels open successully;
   /// stricty this is true if none fail, so if no channels are registed this
   /// function always returns true.
   ///
   bool openRegisteredChannels ();

   /// Conveniance functions to close all currently registered channels.
   ///
   void closeRegisteredChannels ();

   /// Conveniance functions to delete all currently registered channels.
   ///
   void deleteRegisteredChannels ();

   /// Conveniance function to test all channels ready. For ReadModes
   /// Subscribe (the default) and SingleRead this means dataIsAvailable()
   /// is true, while for read mode NoRead the test is isConnected() is true.
   ///
   bool areAllRegisteredChannelsReady () const;

   /// This function performs a delay poll cycle until either all the channels
   /// are ready or the total delay time exceedsthe specified timeout.
   /// Returns true if all channels are currently connected.
   /// The timeOut and pollInterval are specified in seconds.
   /// The pollInterval is constrained to be >= 0.001s (1 mSec).
   ///
   bool waitAllRegisteredChannelsReady (const double timeOut, const double pollInterval = 0.05);

   /// Iterates over all registered clients and invokes the specified function.
   /// The iteration order is currently arbitary, and depends upon the embedded
   /// ACAI::Client_Set class.
   ///
   /// Note: The specified function may safely register and/or deregister itself
   /// and/or other clients.
   ///
   void iterateRegisteredChannels (ACAI::IteratorFunction func, void* context = NULL);

protected:
   /// This is a hook functions. It should not / can not be called from outside
   /// of ACAI, but may be overriden by ACAI::Abstract_Client_User sub classes to
   /// allow them to handle connections updates for all registered ACAI::Clients.
   ///
   // Called by ACAI::Client::callConnectionUpdate
   //
   virtual void connectionUpdate (ACAI::Client* sender, const bool isConnected);

   /// This is a hook function. It should not / can not be called from outside
   /// of ACAI, but may be overriden by Abstract_Client_User sub classes to allow
   /// them to handle event updates for all registered ACAI::Clients.
   ///
   // Called by ACAI::Client::callDataUpdate
   //
   virtual void dataUpdate (ACAI::Client* sender, const bool firstUpdate);

   /// This is a hook function. It should not / can not be called from outside
   /// of ACAI, but may be overriden by Abstract_Client_User sub classes to allow
   /// them to handle put callback notifications for all registered ACAI::Clients.
   ///
   // Called by ACAI::Client::callPutCallbackNotifcation
   //
   virtual void putCallbackNotifcation (ACAI::Client* sender, const bool isSuccessful);

private:
   // Make objects of this class non-copyable.
   //
   Abstract_Client_User(const Abstract_Client_User&) {}
   Abstract_Client_User& operator=(const Abstract_Client_User&) { return *this; }

   // Similar to deregisterClient, but DOES NOT call client deregisterUser.
   // Called by a ACAI::Client object from its destructor.
   //
   void removeClientFromList (Client* client);

   // The collection of registered clients.
   //
   ACAI::Client_Set* registeredClients;

   friend class ACAI::Client;
};

}

#endif   // ACAI_ABSTRACT_CLIENT_USER_H_
