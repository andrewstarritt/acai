/* $File: //depot/sw/epics/acai/acaiSup/acai_client_set.h $
 * $Revision: #7 $
 * $DateTime: 2015/10/31 16:08:49 $
 * $Author: andrew $
 *
 * This file is part of the ACAI library. It provides a basic client container.
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

#ifndef ACAI__CLIENT_SET_H_
#define ACAI__CLIENT_SET_H_

#include <set>
#include <acai_client.h>

namespace ACAI {

class Abstract_Client_User;     // differed declaration

/// iterateChannels function signature.
//
typedef void (*IteratorFunction) (ACAI::Client* client, void* context);

// Client_Depot ??

/// \brief The ACAI::Client_Set class provides a simple client reference (or pointer) container.
///
/// At construction time, a container instance may be optionally configured to
/// perform a deep clear when the container object is deleted.
///
/// The class provides the capability to perform an un-ordered general purpose
/// iteration over the clients in the container calling a user function for each set member.
/// The class also provids a number of pre-configured interations.
///
/// NOTE: The class provides no mechanism to ensure that when an ACAI::Client class
/// object is deleted that it is removed from any or all containers which contain
/// a reference to the object. It is responsibilty of the application to ensure that
/// the class set objects contains no dangling references, or to be more precise
/// ensure no operation is performed on a container that would result in the
/// attempt to reference a deleted class object.
///
class ACAI_SHARED_CLASS Client_Set {
public:
   /// Creates a ACAI::Client* container.
   /// If deepDestruction parameter is true, then all client objects in the container
   /// will be deleted just before this object is deleted.
   ///
   explicit Client_Set (const bool deepDestruction = false);

   /// Deletes a ACAI::Client* container.
   /// If deepDestruction was set true during construction then all client objects
   /// in the container will also be deleted
   virtual ~Client_Set ();

   /// Inserts the specified item into the container.
   /// Whilst a container may only contain one instance of a particular client,
   /// there is no restriction on same client being inserted into multiple
   /// containers.
   /// 
   void insert (ACAI::Client* item);

   /// Removes the specified client from the container if it has been previously
   /// inserted. This function never deletes the referenced client object, even
   /// if deepDestruction set true.
   ///
   void remove (ACAI::Client* item);

   /// Returns the count if, i.e. the number of items in, the container.
   ///
   int count () const;

   /// Removes all items from the container.
   /// This function does not delete the clients, even if deepDestruction set true.
   ///
   void clear ();

   /// Removes AND deletes all items in the container.
   /// This function always deletes the clients, even if deepDestruction set false.
   ///
   void deepClear ();
   
   /// \brief Iterates over all clients in the container and invokes the specified function.
   ///
   /// The iteration order is currently arbitary, and depends upon the underlying
   /// container class which may change.
   ///
   /// Note: The iterateChannels function creates and iterates over a copy of the
   /// client container, therefore the iterator function may safely insert and/or
   /// remove elements from the set.
   ///
   void iterateChannels (ACAI::IteratorFunction func, void* context = NULL);

   /// Conveniance functions to open all channels. openAllChannels returns true
   /// if all channels open successully; stricty true if none fail, so an empty
   /// set always returns true.
   ///
   bool openAllChannels ();
   
   /// Conveniance functions to close all channels.
   ///
   void closeAllChannels ();

   /// Registers all the clients with the specified client user.
   ///
   void registerAllClients (ACAI::Abstract_Client_User* user);
   
   /// Deregisters all the clients from the specified client user.
   ///
   void deregisterAllClients (ACAI::Abstract_Client_User* user);

private:
   // Make non-copyable.
   //
   Client_Set(const Client_Set&) {}
   Client_Set& operator=(const Client_Set&) { return *this; }

   typedef std::set<ACAI::Client*> ClientSets;
   ClientSets clientList;
   bool deepDestruction;
};

}

#endif  // ACAI__CLIENT_SET_H_
