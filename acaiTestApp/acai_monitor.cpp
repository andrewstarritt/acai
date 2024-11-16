// acai_monitor.cpp
//
// This is a simple command line program that uses the ACAI library.
// This program mimics some of the features of the EPICS base programs
// camonitor, caget and cainfo.
// This program is intended as example and test of the ACAI library rather
// than as a replacement for the afore mentioned programs.
//
// Copyright (C) 2013-2024  Andrew C. Starritt
//
// The ACAI library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The ACAI library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the ACAI library.  If not, see <http://www.gnu.org/licenses/>.
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
static volatile bool longString = false;
static volatile bool fullArray = false;
static int maxPvNameLength = 0;

static ACAI::Client_Set* clientSet = NULL;


//------------------------------------------------------------------------------
//
static bool getEventMask (const char* mask, ACAI::EventMasks& eventMask)
{
    bool result = true;   // hypothesize all okay

    unsigned uintMask = eventMask = ACAI::EventNone;

    for (int j = 0; mask[j] != '\0'; j++) {
       char c = mask[j];
       switch (c) {
          case 'v':
             uintMask |= ACAI::EventValue;
             break;

          case 'l':
             uintMask |= ACAI::EventArchive;
             break;

          case 'a':
             uintMask |= ACAI::EventAlarm;
             break;

          case 'p':
             uintMask |= ACAI::EventProperty;
             break;

          default:
             std::cerr << "acai_monitor: invalid event mask character '" << c
                       << "' out of \"" << mask << '"' << std::endl;
             result = false;
             break;
       }
       if (!result) break;  // C++ could do with Ada style loop breaks
    }

    eventMask = static_cast<ACAI::EventMasks>(uintMask);
    return result;
}


//------------------------------------------------------------------------------
//
static void showLimits (ACAI::Client* client)
{
   // All numerical values display high (top) to low (bottom)
   std::cout << "   hopr: "  << client->upperDisplayLimit() << std::endl;
   std::cout << "   lopr: "  << client->lowerDisplayLimit() << std::endl;
   std::cout << "   drvh: "  << client->upperControlLimit() << std::endl;
   std::cout << "   drvl: "  << client->lowerControlLimit() << std::endl;
   std::cout << "   hihi: "  << client->upperAlarmLimit() << std::endl;
   std::cout << "   high: "  << client->upperWarningLimit() << std::endl;
   std::cout << "   low:  "  << client->lowerWarningLimit() << std::endl;
   std::cout << "   lolo: "  << client->lowerAlarmLimit() << std::endl;
}

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
         std::cout << "   data: " << client->dataElementCount() << std::endl;

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
               std::cout << "   egu:  "  << client->units() << std::endl;
               std::cout << "   prec: "  << client->precision() << std::endl;
               showLimits (client);
               break;

            case ACAI::ClientFieldCHAR:
            case ACAI::ClientFieldSHORT:
            case ACAI::ClientFieldLONG:
               std::cout << "   egu:  "  << client->units() << std::endl;
               showLimits (client);
               break;

            default:
               break;
         }

         std::cout << std::endl;
      }

      if (!firstupdate || (client->readMode() != ACAI::Subscribe)) {
         std::cout << std::left << std::setw (maxPvNameLength) << client->pvName () << "  ";
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
// Output help information.
//
static void help ()
{
   std::cout
         << "acai_monitor is a simple command line program that uses the ACAI library."<<  std::endl
         << "The program mimics some of the features of the EPICS base camonitor program,"<<  std::endl
         << "and is intended as an example and test of the ACAI library rather than as a"<<  std::endl
         << "replacement for the afore mentioned camonitor program."<< std::endl
         << "" << std::endl
         << "usage: acai_monitor [OPTIONS] PV_NAMES..." << std::endl
         << "       acai_monitor -h | --help" << std::endl
         << "       acai_monitor -v | --version" << std::endl
         << "" << std::endl
         << "Options:" << std::endl
         << "" << std::endl
         << "-m,--meta        show meta information, e.g precision, egu, enum values." << std::endl
         << "" << std::endl
         << "-g,--get         only do gets, as opposed to monitoring." << std::endl
         << "" << std::endl
         << "-mg,-gm          combines -m and -g options." << std::endl
         << "" << std::endl
         // directly cribbed from camonitor -h
         << "-e,--event mask  specify CA event mask to use. <mask> is any combination of" << std::endl
         << "                 'v' (value), 'a' (alarm), 'l' (log/archive), 'p' (property)." << std::endl
         << "                 The default event mask is 'va'." << std::endl
         << "" << std::endl
         << "-l,--longstr     process PV as a long string (if we can)." << std::endl
         << "" << std::endl
         << "-f,--fullarray   specify acai_monitor request all the elements of an array," << std::endl
         << "                 otherwise request only the defined elements. " << std::endl
         << "                 For older versions of EPICS you should always specify this. " << std::endl
         << "" << std::endl
         << "-v,--version     show version information and exit." << std::endl
         << "" << std::endl
         << "-h,--help        show this help message and exit." << std::endl
         << std::endl;
}

//------------------------------------------------------------------------------
// Output version information.
//
static void version ()
{
   std::cout
         << ACAI_VERSION_STRING << " using " EPICS_VERSION_STRING
         << ", CA Protocol version " << ACAI::Client::protocolVersion()
         << std::endl
         << std::endl;
}

//------------------------------------------------------------------------------
//
int main (int argc, char* argv [])
{
   const char* mask = "va";    // set default

   // Process options
   //
   while (argc >= 2) {
      char* p1 = argv[1];

      if ((strcmp (p1, "--help") == 0) || (strcmp (p1, "-h") == 0)) {
         help ();
         return 0;

      } else if ((strcmp (p1, "--version") == 0) || (strcmp (p1, "-v") == 0)) {
         version ();
         return 0;

      } else if ((strcmp (p1, "--meta") == 0) || (strcmp (p1, "-m") == 0)) {
         outputMeta = true;
         argc--;
         argv++;

      } else if ((strcmp (p1, "--get") == 0) || (strcmp (p1, "-g") == 0)) {
         onlyDoGets = true;
         argc--;
         argv++;

      } else if ((strcmp (p1, "-mg") == 0) || (strcmp (p1, "-gm") == 0)) {
         outputMeta = true;
         onlyDoGets = true;
         argc--;
         argv++;

      } else if ((strcmp (p1, "--event") == 0) || (strcmp (p1, "-e") == 0)) {
         if (argc >= 3) {
            mask = argv[2];
            argc -= 2;
            argv += 2;
         } else {
            std::cerr << "acai_monitor: missing mask option " << std::endl;
            return 1;
         }

      } else if ((strcmp (p1, "--longstr") == 0) || (strcmp (p1, "-l") == 0)) {
         longString = true;
         argc--;
         argv++;

      } else if ((strcmp (p1, "--fullarray") == 0) || (strcmp (p1, "-f") == 0)) {
         fullArray = true;
         argc--;
         argv++;

      } else if (p1[0] == '-') {
         // No sensible PV name starts with a hyphen, so assume a bad option.
         //
         std::cerr << "acai_monitor: error: no such option: " << p1 << std::endl;
         return 1;

      } else {
         // not an option, so must be 1st parameter
         //
         break;
      }
   }

   if (argc < 2) {
      std::cerr << "acai_monitor: No PV name(s) specified" << std::endl;
      return 2;
   }

   ACAI::EventMasks eventMask = ACAI::EventNone;
   bool status = getEventMask (mask, eventMask);
   if (!status) return 1;

   if (eventMask == ACAI::EventNone) {
      std::cerr << "acai_monitor: null event specified" <<  std::endl;
      return 1;
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
      client->setEventMask (eventMask);
      client->setUpdateHandler (dataUpdateEventHandlers);
      client->setLongString (longString);
      if (!fullArray) {
         client->setRequestCount (0);   // for newer clients only !!
      }
      clientSet->insert (client);
      int len = strnlen(argv [j], 80);
      if (maxPvNameLength < len) {
         maxPvNameLength = len;
      }
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
      ACAI::Client::poll ();     // call back functions invoked from here
   }

   clientSet->closeAllChannels ();
   ACAI::Client::poll ();

   delete clientSet;             // performs a deepClear
   ACAI::Client::finalise ();

   return 0;
}

// end
