// acai_monitor.cpp
//
// This is a simple command line programs that uses the ACAI library.
// This program mimics some of the features of the EPICS base program camonitor.
// This program is intended as example and test of the ACAI library rather
// than as a replacement for the afore mentioned camonitor program.
//
// Copyright (C) 2015-2019  Andrew C. Starritt
//
// The acai_monitor program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The acai_monitor program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License and
// the Lesser GNU General Public License along with the acai_monitor program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Contact details:
// andrew.starritt@gmail.com
// PO Box 3118, Prahran East, Victoria 3181, Australia.
//

#include <iostream>
#include <iomanip>
#include <string.h>
#include <signal.h>
#include <acai_client_types.h>
#include <acai_client.h>
#include <acai_client_set.h>
#include <acai_version.h>
#include <epicsThread.h>
#include <epicsVersion.h>

static volatile bool outputMeta = false;
static volatile bool onlyDoGets = false;
static ACAI::Client_Set* clientSet = NULL;

//------------------------------------------------------------------------------
//
static void dataUpdateEventHandlers (ACAI::Client* client, const bool firstupdate)
{
   if (client) {
      // On connection, we get the read response (firstupdate true) immediately
      // followed by first subscription update (firstupdate false).
      // Don't need do a double output.
      //
      if (firstupdate && outputMeta) {
         int n = 0;
         std::cout << client->pvName () << ":" << std::endl;
         std::cout << "   host: " << client->hostName() << std::endl;
         std::cout << "   type: " << ACAI::clientFieldTypeImage (client->hostFieldType()) << std::endl;
         std::cout << "   nelm: " << client->hostElementCount() << std::endl;

         switch (client->dataFieldType()) {
            case ACAI::ClientFieldSTRING:
               break;

            case ACAI::ClientFieldENUM:
               n = client->enumerationStatesCount ();
               for (int j = 0; j < n; j++) {
                  std::cout << "   [" << j << "/" << n << "] "
                            << client->getEnumeration (j) << std::endl;
               }
               break;

            case ACAI::ClientFieldFLOAT:
            case ACAI::ClientFieldDOUBLE:
               std::cout << "   prec: "  << client->precision() << std::endl;
               // fall through
            case ACAI::ClientFieldCHAR:
            case ACAI::ClientFieldSHORT:
            case ACAI::ClientFieldLONG:
               std::cout << "   egu:  "  << client->units() << std::endl;
               std::cout << "   lopr: "  << client->lowerDisplayLimit() << std::endl;
               std::cout << "   hopr: "  << client->upperDisplayLimit() << std::endl;
               std::cout << "   drvl: "  << client->lowerControlLimit() << std::endl;
               std::cout << "   drvh: "  << client->upperControlLimit() << std::endl;
               std::cout << "   low:  "  << client->lowerWarningLimit() << std::endl;
               std::cout << "   high: "  << client->upperWarningLimit() << std::endl;
               std::cout << "   lolo: "  << client->lowerAlarmLimit() << std::endl;
               std::cout << "   hihi: "  << client->upperAlarmLimit() << std::endl;
               break;

            default:
               break;
         }

         std::cout << std::endl;
      }

      if (!firstupdate || (client->readMode() != ACAI::Subscribe)) {
         std::cout << std::left << std::setw (40) << client->pvName () << " ";
         std::cout << client->localTimeImage (3) << " ";

         if (client->processingAsLongString ()) {
             std::cout << " " << client->getString ();
         } else {
            const unsigned int n = client->dataElementCount ();
            if (n > 1) {
               std::cout << "[" << n << "]";
            }
            for (unsigned int j = 0; j < n; j++) {
               client->setIncludeUnits (j == (n-1));
               std::cout << " " << client->getString (j);
            }
         }
         std::cout << " " << client->alarmSeverityImage ()
                   << " " << client->alarmStatusImage ();
         std::cout << std::endl;

         if (onlyDoGets) {
            client->closeChannel ();

            if (clientSet)
               clientSet->remove (client);
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
         std::cerr << "Channel connect timed out: " << client->pvName ()
                   << " PV not found" <<  std::endl;
      } else
      if (!client->dataIsAvailable ()) {
         std::cerr << "Channel read failure: " << client->pvName ()
                   << " PV data not available" <<  std::endl;
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
   if (argc >= 2 && (strcmp (argv[1], "--help") == 0 || strcmp (argv[1], "-h") == 0)) {
      std::cout
      << "acai_monitor is a simple command line programs that uses the ACAI library."<<  std::endl
      << "This program mimics some of the features of the EPICS base camonitor program,"<<  std::endl
      << "and is intended as an example and test of the ACAI library rather than as a"<<  std::endl
      << "replacement for the afore mentioned camonitor program."<< std::endl
      << "" << std::endl
      << "usage: acai_monitor [-m|--meta] [-g|--get] PV_NAMES..." << std::endl
      << "       acai_monitor -h | --help" << std::endl
      << "       acai_monitor -v | --version" << std::endl
      << "" << std::endl
      << "Options:" << std::endl
      << "" << std::endl
      << "-m,--meta     show meta information, e.g precision, egu, enum values." << std::endl
      << "" << std::endl
      << "-g,--get      only do gets, as opposed to monitoring." << std::endl
      << "              note: must follow meta option if both options are defined." << std::endl
      << "" << std::endl
      << "-v,--version  show version information and exit." << std::endl
      << "" << std::endl
      << "-g,--get      shiw this help message and exit." << std::endl
      << std::endl;

      return 0;
   }

   if (argc >= 2 && (strcmp (argv[1], "--version") == 0 || strcmp (argv[1], "-v") == 0)) {
      std::cout
      << ACAI_VERSION_STRING << " using " EPICS_VERSION_STRING << std::endl
      << std::endl;

      return 0;
   }


   if (argc >= 2 && (strcmp (argv[1], "--meta") == 0 || strcmp (argv[1], "-m") == 0)) {
      outputMeta = true;
      argc--;
      argv++;
   }

   if (argc >= 2 && (strcmp (argv[1], "--get") == 0 || strcmp (argv[1], "-g") == 0)) {
      onlyDoGets = true;
      argc--;
      argv++;
   }


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
      client = new ACAI::Client (argv [j]);
      client->setReadMode (onlyDoGets ? ACAI::SingleRead : ACAI::Subscribe);
      client->setIncludeUnits (true);
      client->setUpdateHandler (dataUpdateEventHandlers);
      clientSet->insert (client);
   }

   clientSet->openAllChannels ();

   // Run simple event loop for 2 seconds.
   //
   ok = clientSet->waitAllChannelsReady (2.0, 0.02);
   if (!ok) {
      std::cerr << "** Not all channels connected" << std::endl;
   }

   // Check for connection failures.
   //
   if (!shutDownIsRequired ()) {
      clientSet->iterateChannels (reportConnectionFailures, NULL);
   }

   // Resume event loop if in monitor mode.
   //
   while (!shutDownIsRequired () && !onlyDoGets) {
      epicsThreadSleep (0.02);   // 20mSec
      ACAI::Client::poll ();     // call back function invoked from here
   }

   clientSet->closeAllChannels ();
   ACAI::Client::poll ();

   delete clientSet;             // performs a deepClear
   ACAI::Client::finalise ();

   return 0;
}

// end
