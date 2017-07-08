/* acai_client_types.cpp
 *
 * This file is part of the ACAI library.
 *
 * Copyright (C) 2013,2014,2015,2017  Andrew C. Starritt
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

#include <alarm.h>
#include <alarmString.h>
#include <acai_client_types.h>
#include <acai_private_common.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC int ACAI::csnprintf (ACAI::ClientString& target, size_t size, const char* format, ...)
{
   va_list args;
   char buffer [size + 1];
   int result;

   va_start (args, format);
   result = vsnprintf (buffer, sizeof (buffer), format, args);
   va_end (args);

   target = buffer;
   return result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::csnprintf (size_t size, const char* format, ...)
{
   va_list args;
   char buffer [size + 1];

   va_start (args, format);
   vsnprintf (buffer, sizeof (buffer), format, args);
   va_end (args);

   return ACAI::ClientString (buffer);
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::limitedAssign (const char* source, const size_t maxSize)
{
   ACAI::ClientString result;

   // If string size is OK, use it as is
   //
   if (strnlen (source, maxSize) < maxSize) {
      // String has embedded '\0' - so we are okay to use regular constructor.
      //
      result = ACAI::ClientString (source);
   } else {
      // If string size is not OK, use the  "from buffer" constructor to use
      // the initial characters up to the allowed maximum.
      //
      result = ACAI::ClientString (source, maxSize);
   }

   return result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::alarmSeverityImage (const ACAI::ClientAlarmSeverity severity)
{
   ClientString result;
   const int n = int (severity);
   if ((n >= 0) && (n < ALARM_NSEV)) {
      result = ACAI::ClientString (epicsAlarmSeverityStrings[n]);
   } else if (n == ClientDisconnected) {
      result = ACAI::ClientString ("DISCONNECTED");
   } else {
      result = ACAI::csnprintf (40, "unknown severity %d", n);
   }

   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::alarmStatusImage (const ACAI::ClientAlarmCondition status)
{
   ClientString result;
   const int n = int (status);
   if ((n >= 0) && (n < ALARM_NSTATUS)) {
      result = ACAI::ClientString (epicsAlarmConditionStrings[n]);
   } else {
      result = ACAI::csnprintf (40, "unknown status %d", n);
   }
   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC time_t ACAI::utcTimeOf (const ACAI::ClientTimeStamp& ts, int* nanoSecOut)
{
   // EPICS timestamp epoch: This is Mon Jan  1 00:00:00 1990 UTC.
   //
   // This itself is expressed as a system time which represents the number
   // of seconds elapsed since 00:00:00 on January 1, 1970, UTC.
   //
   static const time_t epics_epoch = 631152000;

   if (nanoSecOut) {
      *nanoSecOut = ts.nsec;
   }

   return ts.secPastEpoch + epics_epoch;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::utcTimeImage (const ACAI::ClientTimeStamp& ts,
                                                        const int precision)
{
   static const int scale [10] = {
      1000000000, 100000000, 10000000, 1000000,
      100000, 10000, 1000, 100, 10, 1
   };

   ClientString result;

   // Convert time
   //
   int nanoSec;
   const time_t utc = ACAI::utcTimeOf (ts, &nanoSec);

   // Make maybe uninitialised warning go away.
   //
   struct tm bt;
   bt.tm_year = bt.tm_mon = bt.tm_mday = bt.tm_hour = bt.tm_min = bt.tm_sec = 0;

   // Form broken-down UTC time bt
   //
   gmtime_r (&utc, &bt);

   // In broken-down time, tm_year is the number of years since 1900,
   // and January is month 0.
   //
   char text [40];
   snprintf (text, 40, "%04d-%02d-%02d %02d:%02d:%02d",
             1900 + bt.tm_year, 1 + bt.tm_mon, bt.tm_mday,
             bt.tm_hour, bt.tm_min, bt.tm_sec);

   result = text;

   // Add precision if required.
   //
   if (precision > 0) {
      char format[8];
      char fraction[16];

      const int p = MIN (precision, 9);
      sprintf (format, ".%%0%dd", p);
      const int f =  nanoSec / scale[p];
      sprintf (fraction, format, f);
      result.append (fraction);
   }

   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::clientFieldTypeImage (const ACAI::ClientFieldType cft)
{
   ClientString result;

   switch (cft) {
      case ACAI::ClientFieldSTRING:    result = "STRING";    break;
      case ACAI::ClientFieldSHORT:     result = "SHORT";     break;
      case ACAI::ClientFieldFLOAT:     result = "FLOAT";     break;
      case ACAI::ClientFieldENUM:      result = "ENUM";      break;
      case ACAI::ClientFieldCHAR:      result = "CHAR";      break;
      case ACAI::ClientFieldLONG:      result = "LONG";      break;
      case ACAI::ClientFieldDOUBLE:    result = "DOUBLE";    break;
      case ACAI::ClientFieldNO_ACCESS: result = "NO_ACCESS"; break;
      case ACAI::ClientFieldDefault:   result = "Default";   break;
      default:
         csnprintf (result, 40, "Unknown field type (%d)", (int) cft);
         break;
   }
   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC int ACAI::version ()
{
   return ACAI_VERSION;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC ACAI::ClientString ACAI::versionString ()
{
   return ACAI_VERSION_STRING;
}

// end

