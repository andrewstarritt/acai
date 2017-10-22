// acai_monitor.cpp
//
// The is a simple command line programs that uses the ACAI framework.
// This programs mimics the EPICS base program camonitor.
// This program is intended as example (and test) of the ACAI framework
// rather than as replacements for the afore mentioned camonitor program.
//

#include <iostream>
#include <iomanip>
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
         std::cout << std::left << std::setw (40) << client->pvName ();
         const int n = client->enumerationStatesCount ();
         if (n > 0) {
            std::cout << " enum values:" << std::endl;
            for (int j = 0; j < n; j++) {
               std::cout << " [" << j << "/" << n << "] " << client->getEnumeration (j) <<  std::endl;
            }
         } else {
            std::cout << " egu: " << client->units()
                      << ", prec: " << client->precision () << std::endl;
         }
      }

      if (!firstupdate || (client->readMode() != ACAI::Subscribe)) {
         std::cout << std::left << std::setw (40) << client->pvName () << " ";
         std::cout << client->localTimeImage (3) << " ";

         if (client->processingAsLongString ()) {
            std::cout <<  " " << client->getString () << std::endl;
         } else {
            const unsigned int n = client->dataElementCount ();
            if (n > 1) {
               std::cout << "[" << n << "]";
            }
            for (unsigned int j = 0; j < n; j++) {
               std::cout <<  " " << client->getString (j);
            }
            std::cout << std::endl;
         }
      }
   } else {
      std::cerr << "acai_monitor: null client" <<  std::endl;
   }
}

//------------------------------------------------------------------------------
//
static void reportConnectionFailures (ACAI::Client* client, void* /* context */ )
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
   ok = clientSet->waitAllChannelsReady (2.0, 0.02);
   std::cerr << "waitAllChannelsReady: "
             << (ok ? "true " : "false" ) << std::endl;

   // Check for connection failures.
   //
   if (!shutDownIsRequired ()) {
      clientSet->iterateChannels (reportConnectionFailures, NULL);
   }

   // Resume event loop.
   //
   while (!shutDownIsRequired ()) {
      epicsThreadSleep (0.02);   // 20mSec
      ACAI::Client::poll ();     // call back function invoked from here
   }

   clientSet->closeAllChannels ();
   ACAI::Client::poll ();

   ACAI::Client::finalise ();
   delete clientSet;             // performs a deepClear

   return 0;
}

// end
