// $File: //depot/sw/epics/acai/acaiTestApp/acai_monitor.cpp $
// $Revision: #6 $
// $DateTime: 2015/10/31 16:08:49 $
// Last checked in by: $Author: andrew $
//

#include <iostream>
#include <signal.h>
#include <acai_client_types.h>
#include <acai_client.h>
#include <acai_client_set.h>
#include <epicsThread.h>

//------------------------------------------------------------------------------
//
static void dataUpdateEventHandlers (ACAI::Client* client, const bool firstupdate)
{
   if (client) {
      // On connection, we get the read response (firstupdate true) immediately
      // followed by first subscription update (firstupdate false).
      // Don't need do a double output.
      //
      if (firstupdate) {
         int n = client->enumerationStatesCount();
         for (int j = 0; j < n; j++) {
            std::cout << "[" << j << "/" << n << "] " << client->getEnumeration (j) <<  std::endl;
         }
      }
      if (!firstupdate || (client->readMode() != ACAI::Subscribe)) {
         std::cout << client->pvName () << " ";

         unsigned int n = client->dataElementCount ();
         if (client->processingAsLongString ()) n = 1;

         if (n > 1) {
            std::cout << "[" << n << "]";
         }
         for (unsigned int j = 0; j < n; j++) {
            std::cout << " " << client->getString (j);
         }
         std::cout << std::endl;
      }
   } else {
      std::cerr << "acai_monitor: null client" <<  std::endl;
   }
}

//------------------------------------------------------------------------------
//
static void reportConnectionFailures (ACAI::Client* client, void* context)
{
   if (client) {
      if (!client->isConnected ()) {
         std::cerr << "Channel connect timed out: " << client->pvName () << " PV not found" <<  std::endl;
      } else
      if (!client->dataIsAvailable ()) {
         std::cerr << "Channel read failure: " << client->pvName () << " PV data not available" <<  std::endl;
      }
   } else {
      std::cerr << "acai_monitor: null client" <<  std::endl;
   }
}

static volatile bool sigIntReceived = false;
static volatile bool sigTermReceived = false;

//------------------------------------------------------------------------------
//
static void signalCatcher (int sig)
{
   switch (sig) {
      case SIGINT:
         std::cerr << "\nSIGINT received - initiating orderly shutdown." << std::endl;
         sigIntReceived = true;
         break;

      case SIGTERM:
         std::cerr << "\nSIGTERM received - initiating orderly shutdown." << std::endl;
         sigTermReceived = true;
         break;
   }
}

//------------------------------------------------------------------------------
//
static void signalSetup ()
{
   signal (SIGTERM, signalCatcher);
   signal (SIGINT, signalCatcher);
}

//------------------------------------------------------------------------------
// Checks if time to shut down the monitor program.
// This a test if SIGINT/SIGTERM have been received.
//
static bool shutDownIsRequired ()
{
   return sigIntReceived || sigTermReceived;
}


//------------------------------------------------------------------------------
//
int main (int argc, char* argv [])
{
   ACAI::Client_Set* clientSet = NULL;

   if (argc < 2) {
      std::cerr << "acai_monitor: No PV name(s) specified" <<  std::endl;
      return 2;
   }

   bool ok = ACAI::Client::initialise ();
   if (!ok) {
      std::cerr << "ACAI::Client::initialise failed." <<  std::endl;
      return 2;
   }

   signalSetup ();

   clientSet = new ACAI::Client_Set (true);

   for (int j = 1; j < argc; j++) {
      ACAI::Client* client;
      client = new  ACAI::Client (argv [j]);
      client->setReadMode (ACAI::Subscribe);   // default
      client->setIncludeUnits (true);
      client->setUpdateHandler (dataUpdateEventHandlers);
      clientSet->insert (client);
   }

   clientSet->openAllChannels ();

   // Run simple event loop for 2 seconds.
   //
   ACAI::Client::poll ();
   for (int t = 0; (t < 100) && (!shutDownIsRequired ()); t++) {
      epicsThreadSleep (0.02);   // 20mSec
      ACAI::Client::poll ();     // call back function invked from here
   }

   // Check for connection failures.
   //
   if (!shutDownIsRequired ())
      clientSet->iterateChannels (reportConnectionFailures, NULL);

   // Resume event loop.
   //
   while (!shutDownIsRequired ()) {
      epicsThreadSleep (0.02);   // 20mSec
      ACAI::Client::poll ();     // call back function invked from here
   }

   clientSet->closeAllChannels ();
   ACAI::Client::poll ();

   ACAI::Client::finalise ();
   delete clientSet;             // performs a deepClear

   return 0;
}

// end
