/* $File: //depot/sw/epics/acai/acaiSup/acai_abstract_client_user.h $
 * $Revision: #8 $
 * $DateTime: 2015/06/22 20:47:28 $
 * $Author: andrew $
 *
 * This file is part of the ACAI library. It provides a base class that
 * supports application classes that use this library.
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

#ifndef ACAI__ABSTRACT_CLIENT_USER_H_
#define ACAI__ABSTRACT_CLIENT_USER_H_

#include <acai_client_types.h>
#include <acai_client_set.h>

namespace ACAI {

class Client;       // differed declaration.

/// \brief The ACAI::Abstract_Client_User is a base class provided to support
/// application classes that use the ACAI library.
///
/// An instance of a class derived from the ACAI::Abstract_Client_User class may
/// register zero, one or more ACAI::Client or derived class objects.
/// This means that when one of those registered objects connects/diconnects or
/// receives an event update, it will invoke the connectionUpdate or dataUpdate
/// virtual function of this object.
///
/// It is intended that appications derive their own classes using this class as a
/// base class. A number of functions have been declared virtual specifically to
/// support this.
/// The virtual functions are Abstract_Client_User::~Abstract_Client_User,
/// Abstract_Client_User::connectionUpdate and Abstract_Client_User::dataUpdate.
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
   /// Constructor - initiliases the object.
   ///
   explicit Abstract_Client_User ();

   /// NOTE: All clients are automatically degreistered when this object is deleted.
   ///
   virtual ~Abstract_Client_User ();

   /// Register this user for conection/update events associated with the nominated
   /// client.
   ///
   void registerClient (ACAI::Client* client);

   /// Deregister this user with the nominated client.
   ///
   /// NOTE: Any client is automatically degreistered if/when the client object
   /// is deleted.
   ///
   void deregisterClient (ACAI::Client* client);

   /// This function opens all currently registered channels.
   /// The openRegisteredChannels returns true if all channels open successully;
   /// stricty this is true if none fail, so if no channels are registed this
   /// function always returns true.
   ///
   bool openRegisteredChannels ();

   /// Conveniance functions to close all currently registered channels.
   ///
   void closeRegisteredChannels ();

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
   // Called by Client::callConnectionUpdate
   //
   virtual void connectionUpdate (ACAI::Client* sender, const bool isConnected);

   /// This is a hook function. It should not / can not be called from outside
   /// of ACAI, but may be overriden by Abstract_Client_User sub classes to allow
   /// them to handle event updates for all registered ACAI::Clients.
   ///
   // Called by Client::callDataUpdate
   //
   virtual void dataUpdate (ACAI::Client* sender, const bool firstUpdate);

private:
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

#endif   // ACAI__ABSTRACT_CLIENT_USER_H_
