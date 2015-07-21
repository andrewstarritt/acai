// $File: //depot/sw/epics/acai/acaiTestApp/acai_monitor.cpp $
// $Revision: #4 $
// $DateTime: 2015/06/22 20:49:29 $
// Last checked in by: $Author: andrew $
//

#include <iostream>
#include <acai_client_types.h>
#include <acai_client.h>
#include <acai_client_set.h>
#include <epicsThread.h>

//------------------------------------------------------------------------------
//
static void dataUpdateEventHandlers (ACAI::Client* client, const bool firstupdate)
{
   if (client) {
      // On connection, we get the read response (firstupdate true) immediatly
      // followed by first subscription update (firstupdate false).
      // Don't need do a double output.
      //
      if (!firstupdate || (client->readMode() != ACAI::Subscribe)) {
         std::cout << client->pvName () << " ";
         const unsigned int n = client->dataElementCount ();
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

//------------------------------------------------------------------------------
//
int main (int argc, char* argv [])
{
   ACAI::Client_Set* clientSet = NULL;

   if (argc < 2) {
      std::cerr << "acai_monitor: No PV name(s) specified" <<  std::endl;
      return 2;
   }

   ACAI::Client::initialise ();
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
   for (int t = 0; t < 100; t++) {
      epicsThreadSleep (0.02);  // 20mSec
      ACAI::Client::poll ();   // call back function invked from here
   }

   clientSet->iterateChannels (reportConnectionFailures, NULL);

   while (true) {
      epicsThreadSleep (0.02);
      ACAI::Client::poll ();   // call back function invked from here
   }

   clientSet->closeAllChannels ();
   ACAI::Client::poll ();

   ACAI::Client::finalise ();
   delete clientSet;

   return 0;
}

// end
