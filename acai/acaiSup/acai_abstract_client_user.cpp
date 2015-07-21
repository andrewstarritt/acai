/* $File: //depot/sw/epics/acai/acaiSup/acai_abstract_client_user.cpp $
 * $Revision: #5 $
 * $DateTime: 2015/06/21 17:41:41 $
 * $Author: andrew $
 *
 * This file is part of the ACAI library.
 *
 * Copyright (c) 2014,2015  Andrew C. Starritt
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

#include <acai_client.h>
#include <acai_client_set.h>
#include <acai_abstract_client_user.h>


//==============================================================================
// ACAI::Abstract_Client_User methods
//==============================================================================
//
ACAI::Abstract_Client_User::Abstract_Client_User ()
{
   this->registeredClients = new ACAI::Client_Set (false);
}

//------------------------------------------------------------------------------
//
ACAI::Abstract_Client_User::~Abstract_Client_User ()
{
   // Deregister all clients.
   //
   this->registeredClients->deregisterAllClients (this);  // calls own deregisterClient
   this->registeredClients->clear ();
   delete this->registeredClients;
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::registerClient (ACAI::Client* client)
{
   if (!client) return;  // sanity check

   // Set up two-way x-reference.
   //
   client->registerUser (this);
   this->registeredClients->insert (client);
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::deregisterClient (ACAI::Client* client)
{
   if (!client) return;  // sanity check

   // Clear two-way x-reference.
   //
   client->deregisterUser (this);
   this->registeredClients->remove (client);
}

//------------------------------------------------------------------------------
//
bool ACAI::Abstract_Client_User::openRegisteredChannels ()
{
   return this->registeredClients->openAllChannels();
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::closeRegisteredChannels ()
{
    this->registeredClients->closeAllChannels();
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::iterateRegisteredChannels (ACAI::IteratorFunction func,
                                                            void* context)
{
   this->registeredClients->iterateChannels (func, context);
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::removeClientFromList (ACAI::Client* client)
{
   if (!client) return;  // sanity check
   // client obect is being deleted.
   this->registeredClients->remove (client);
}

//------------------------------------------------------------------------------
// Place holders, needed as these are not pure virtual.
//
void ACAI::Abstract_Client_User::connectionUpdate (ACAI::Client* /* sender */,
                                                   const bool /* isConnected */)
{
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::dataUpdate (ACAI::Client* /* sender */,
                                             const bool /* firstUpdate */)
{
}

// end