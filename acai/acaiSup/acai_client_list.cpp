/* acai/acaiSup/acai_client_list.cpp
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

#include <acai_abstract_client_user.h>
#include <acai_client_list.h>
#include <acai_private_common.h>


//------------------------------------------------------------------------------
//
ACAI::Client_List::Client_List (const bool deepDestructionIn)
{
   this->deepDestruction = deepDestructionIn;
   this->clear ();
}

//------------------------------------------------------------------------------
//
ACAI::Client_List::~Client_List ()
{
   if (this->deepDestruction) {
      this->deepClear();
   } else {
      this->clear ();
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::append (ACAI::Client* item)
{
   if (item) {
      this->clientList.insert (item);
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::remove (Client* item)
{
   if (item) {
      this->clientList.erase (item);
   }
}

//------------------------------------------------------------------------------
//
int ACAI::Client_List::size () const
{
   return (int) this->clientList.size ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::clear ()
{
   this->clientList.clear ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::deepClear ()
{
   // Delete PV client objects.
   //
   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         delete client;
      }
   }
   this->clear ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::iterateChannels (IteratorFunction func, void* context)
{
   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && func) {
         func (client, context);
      }
   }
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_List::openChannels ()
{
   bool result = true;

   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         result &= client->openChannel ();
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::closeChannels ()
{
   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         client->closeChannel ();
      }
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::registerClients (ACAI::Abstract_Client_User* user)
{
   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && user) {
          user->registerClient (client);
      }
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_List::deregisterClients (ACAI::Abstract_Client_User* user)
{
   ITERATE (ACAI::Client_List::ClientLists, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && user) {
          user->deregisterClient (client);
      }
   }
}

// end
