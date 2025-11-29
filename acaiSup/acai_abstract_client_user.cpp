/* acai_abstract_client_user.cpp
 *
 * This file is part of the ACAI library.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 */

#include "acai_abstract_client_user.h"
#include "acai_client.h"
#include "acai_client_set.h"

//==============================================================================
// Client_Set iterator callback functions.
//==============================================================================
// static
void ACAI::Abstract_Client_User::registerOneClient (ACAI::Client* client, void* context)
{
   ACAI::Abstract_Client_User* self = reinterpret_cast<ACAI::Abstract_Client_User*>(context);

   if (!client) return;  // sanity check
   if (!self) return;    // sanity check

   self->registerClient (client);
}

//------------------------------------------------------------------------------
// static
void ACAI::Abstract_Client_User::deregisterOneClient (ACAI::Client* client, void* context)
{
   ACAI::Abstract_Client_User* self = reinterpret_cast<ACAI::Abstract_Client_User*>(context);

   if (!client) return;  // sanity check
   if (!self) return;    // sanity check

   self->deregisterClient (client);
}

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
   this->deregisterAllClients (this->registeredClients);
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
void ACAI::Abstract_Client_User::registerAllClients (ACAI::Client_Set* clientSet)
{
   if (!clientSet) return;  // sanity check
   clientSet->iterateChannels (ACAI::Abstract_Client_User::registerOneClient, this);
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
void ACAI::Abstract_Client_User::deregisterAllClients (ACAI::Client_Set* clientSet)
{
   if (!clientSet) return;  // sanity check
   clientSet->iterateChannels (ACAI::Abstract_Client_User::deregisterOneClient, this);
}

//------------------------------------------------------------------------------
//
bool ACAI::Abstract_Client_User::clientIsRegistered (ACAI::Client* client) const
{
   return this->registeredClients->contains (client);
}

//------------------------------------------------------------------------------
//
bool ACAI::Abstract_Client_User::openRegisteredChannels ()
{
   return this->registeredClients->openAllChannels ();
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::closeRegisteredChannels ()
{
   this->registeredClients->closeAllChannels ();
}

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::deleteRegisteredChannels ()
{
   this->registeredClients->deepClear ();
}

//------------------------------------------------------------------------------
//
bool ACAI::Abstract_Client_User::areAllRegisteredChannelsReady () const
{
   return this->registeredClients->areAllChannelsReady ();
}

//------------------------------------------------------------------------------
//
bool ACAI::Abstract_Client_User::waitAllRegisteredChannelsReady (const double timeOut,
                                                                 const double pollInterval)
{
   return this->registeredClients->waitAllChannelsReady (timeOut, pollInterval);
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

//------------------------------------------------------------------------------
//
void ACAI::Abstract_Client_User::putCallbackNotifcation (ACAI::Client* /* sender */,
                                                         const bool /* isSuccessful */)
{
}

// end
