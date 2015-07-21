/* acai/acaiSup/acai_client_list.h
 *
 * This file is part of the ACAI library.
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

#ifndef ACAI__CLIENT_LIST_H_
#define ACAI__CLIENT_LIST_H_

#include <set>
#include <acai_client.h>

namespace ACAI {

class Abstract_Client_User;     // differed declaration

/// This class provides a simple client container. The class also provides an un-ordered
/// iteration over the clients in the container capabity calling a user function together
/// with a number of pre-configured interators.
///
class ACAI_SHARED_CLASS Client_List {
public:
   // If deepDestruction parameter is true, then all clients object in the list will be
   // deleted just before this object is deleted. It is the callers responsibilty to
   // ensure no danfling references are left elsewhere.
   //
   explicit Client_List (const bool deepDestruction = false);
   virtual ~Client_List ();

   void append (Client* item);  // no restriction on same client being added multiple times or to multiple lists.
   void remove (Client* item);  // does not delete client
   int size () const;

   void clear ();           // just clears the internal list
   void deepClear ();       // deletes all internal items and clears the list

   // Iterators over all clients in the list and invokes the specified function.
   // The iteration order is currently arbitary, and depends upon the underlying
   // container class which may change.
   //
   typedef void (*IteratorFunction) (ACAI::Client* client, void* context);
   void iterateChannels (IteratorFunction func, void* context = NULL);

   // Conveniance functions to open/close all channels. openChannels returns true
   // if all channels open successully; stricty true if none fail, so an empty
   // list always returns true.
   //
   bool openChannels ();
   void closeChannels ();   // close all channels

   // Registers/deregisters all list clients with the specified client user.
   //
   void registerClients (ACAI::Abstract_Client_User* user);
   void deregisterClients (ACAI::Abstract_Client_User* user);

private:
   typedef std::set<Client*> ClientLists;
   ClientLists clientList;
   bool deepDestruction;
};

}

#endif  // ACAI__CLIENT_LIST_H_
