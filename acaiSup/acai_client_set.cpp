/* acai_client_set.cpp
 *
 * This file is part of the ACAI library.
 *
 * Copyright (C) 2014-2018  Andrew C. Starritt
 *
 * The ACAI library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * You can also redistribute the ACAI library and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version when this library is disributed with and as part of the
 * EPICS QT Framework (https://github.com/qtepics).
 *
 * The ACAI library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * the Lesser GNU General Public License along with the ACAI library.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact details:
 * starritt@netspace.net.au
 * PO Box 3118, Prahran East, Victoria 3181, Australia.
 *
 */

#include <acai_client_set.h>
#include <acai_abstract_client_user.h>
#include <acai_private_common.h>
#include <epicsThread.h>


//------------------------------------------------------------------------------
//
ACAI::Client_Set::Client_Set (const bool deepDestructionIn)
{
   this->deepDestruction = deepDestructionIn;
   this->clear ();
}

//------------------------------------------------------------------------------
//
ACAI::Client_Set::~Client_Set ()
{
   if (this->deepDestruction) {
      this->deepClear();
   } else {
      this->clear ();
   }
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_Set::isDeepDestruction () const
{
   return this->deepDestruction;
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::insert (ACAI::Client* item)
{
   if (item) {
      this->clientList.insert (item);
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::remove (ACAI::Client* item)
{
   if (item) {
      this->clientList.erase (item);
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::insertAllClients (ACAI::Client_Set* other)
{
   if (!other) return;

   // Incase some calls:  some_set.insertAllClients (some_set);
   // No so critical as the remove all case
   //
   ACAI::Client_Set::ClientSets copy = other->clientList;

   ACAI_ITERATE (ACAI::Client_Set::ClientSets, copy, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         this->insert (client);
      }
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::removeAllClients (ACAI::Client_Set* other)
{
   if (!other) return;

   // Incase some calls:  some_set.removeAllClients (some_set);
   //
   ACAI::Client_Set::ClientSets copy = other->clientList;

   ACAI_ITERATE (ACAI::Client_Set::ClientSets, copy, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         this->remove (client);
      }
   }
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_Set::contains (ACAI::Client* item) const
{
   bool result = false;
   if (item) {
      result = (this->clientList.find (item) != this->clientList.end ());
   }
   return result;
}

//------------------------------------------------------------------------------
//
int ACAI::Client_Set::count () const
{
   return (int) this->clientList.size ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::clear ()
{
   this->clientList.clear ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::deepClear ()
{
   // Delete PV client objects.
   //
   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         delete client;
      }
   }
   this->clear ();
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::iterateChannels (ACAI::IteratorFunction func, void* context)
{
   // NOTE: Create a copy of the set - this allows the iterator function to safely
   // insert and/or remove elements from the set.
   //
   ACAI::Client_Set::ClientSets copy = this->clientList;

   ACAI_ITERATE (ACAI::Client_Set::ClientSets, copy, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && func) {
         func (client, context);
      }
   }
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_Set::openAllChannels ()
{
   bool result = true;

   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         result &= client->openChannel ();
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::closeAllChannels ()
{
   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client) {
         client->closeChannel ();
      }
   }
}

//------------------------------------------------------------------------------
// Maybe: make a client function.
//
bool ACAI::Client_Set::clientIsReady (ACAI::Client* client) const
{
   bool result = false;

   if (client) {
      ACAI::ReadModes rm = client->readMode ();

      switch (rm) {
         case ACAI::NoRead:
            result = client->isConnected ();
            break;

         case ACAI::SingleRead:
         case ACAI::Subscribe:
            result = client->dataIsAvailable();
            break;

         default:
            // What's best error handling policy here?
            //
            result = client->isConnected ();
            break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_Set::areAllChannelsReady () const
{
   bool result = true;

   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      bool clientReady = this->clientIsReady (client);
      if (!clientReady) {
         result= false;    // Only 1 client need not be ready.
         break;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool ACAI::Client_Set::waitAllChannelsReady (const double timeOut, const double pollInterval)
{
   bool result = this->areAllChannelsReady ();
   double total = 0.0;
   while (!result && (total < timeOut)) {
      epicsThreadSleep (pollInterval);
      total += (pollInterval >= 0.001 ? pollInterval : 0.001);
      ACAI::Client::poll ();
      result = this->areAllChannelsReady ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::registerAllClients (ACAI::Abstract_Client_User* user)
{
   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && user) {
          user->registerClient (client);
      }
   }
}

//------------------------------------------------------------------------------
//
void ACAI::Client_Set::deregisterAllClients (ACAI::Abstract_Client_User* user)
{
   ACAI_ITERATE (ACAI::Client_Set::ClientSets, this->clientList, clientRef) {
      ACAI::Client* client = *clientRef;
      if (client && user) {
          user->deregisterClient (client);
      }
   }
}

// end
