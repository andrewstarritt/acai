/* acai_client_types.cpp
 *
 * This file is part of the ACAI library.
 *
 * Copyright (C) 2013-2023  Andrew C. Starritt
 *
 * The ACAI library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * The ACAI library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the ACAI library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact details:
 * andrew.starritt@gmail.com
 * PO Box 3118, Prahran East, Victoria 3181, Australia.
 *
 */

#include <acai_client_types.h>
#include <acai_private_common.h>
#include <epicsTime.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Importing the severity and status strings from the EPICS libraries on windows
// drives me crazy, so just going to roll my own - includng own pseudo severity.
//
static const char* ownAlarmSeverityStrings[ACAI::CLIENT_ALARM_NSEV] = {
    "NO_ALARM",    "MINOR",       "MAJOR",       "INVALID",
    "DISCONNECTED"
};

static const char* ownAlarmConditionStrings[ACAI::CLIENT_ALARM_NSTATUS] = {
    "NO_ALARM",    "READ",        "WRITE",       "HIHI",
    "HIGH",        "LOLO",        "LOW",         "STATE",
    "COS",         "COMM",        "TIMEOUT",     "HWLIMIT",
    "CALC",        "SCAN",        "LINK",        "SOFT",
    "BAD_SUB",     "UDF",         "DISABLE",     "SIMM",
    "READ_ACCESS", "WRITE_ACCESS"
};


//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
int ACAI::csnprintf (ACAI::ClientString& target,
                     size_t size,
                     const char* format, ...)
{
   int result;
   va_list args;
   va_start (args, format);

   if (size <= 512) {    // 512 is a bit arbitary
      // Just use a stack buffer, but still limit to size +1.
      //
      char buffer [513];
      result = vsnprintf (buffer, size + 1, format, args);   // +1 for null byte
      target = buffer;
   } else {
      // Large-ish, allocate/free work buffer.
      //
      char* buffer = (char*) malloc (size + 1);
      result = vsnprintf (buffer, size + 1, format, args);
      target = buffer;
      free (buffer);
   }

   va_end (args);
   return result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::csnprintf (size_t size,
                                    const char* format, ...)
{
   ACAI::ClientString result;
   va_list args;
   va_start (args, format);

   if (size <= 512) {    // 512 is a bit arbitary
      // Just use a stack buffer, but still limit to size +1.
      //
      char buffer [513];
      vsnprintf (buffer, size + 1, format, args);
      result = ACAI::ClientString (buffer);
   } else {
      // Large-ish, allocate/free work buffer.
      //
      char* buffer = (char*) malloc (size + 1);
      vsnprintf (buffer, size + 1, format, args);
      result = ACAI::ClientString (buffer);
      free (buffer);
   }

   va_end (args);
   return result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::limitedAssign (const char* source,
                                        const size_t maxSize)
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
ACAI_SHARED_FUNC
bool ACAI::alarmSeverityIsValid (const ACAI::ClientAlarmSeverity severity)
{
   return ((severity == ClientSevNone)  ||
           (severity == ClientSevMinor) ||
           (severity == ClientSevMajor));
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::alarmSeverityImage (const ACAI::ClientAlarmSeverity severity)
{
   ClientString result;
   const int isevr = int (severity);
   if ((isevr >= 0) && (isevr < CLIENT_ALARM_NSEV)) {
      result = ACAI::ClientString (ownAlarmSeverityStrings[isevr]);
   } else {
      result = ACAI::csnprintf (40, "unknown severity %d", isevr);
   }

   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::alarmStatusImage (const ACAI::ClientAlarmCondition status)
{
   ClientString result;
   const int istat = int (status);
   if ((istat >= 0) && (istat < CLIENT_ALARM_NSTATUS)) {
      result = ACAI::ClientString (ownAlarmConditionStrings[istat]);
   } else {
      result = ACAI::csnprintf (40, "unknown status %d", istat);
   }
   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
time_t ACAI::utcTimeOf (const ACAI::ClientTimeStamp& ts,
                        int* nanoSecOut)
{
   if (nanoSecOut) {
      *nanoSecOut = ts.nsec;
   }

   // EPICS timestamp epoch: This is Mon Jan  1 00:00:00 1990 UTC.
   //
   // This itself is expressed as a system time which represents the number
   // of seconds elapsed since 00:00:00 on January 1, 1970, UTC.
   // (from epicsTime.h)
   //
   return ts.secPastEpoch + POSIX_TIME_AT_EPICS_EPOCH;
}


//------------------------------------------------------------------------------
//
typedef tm* (*GetBrokenDownTime) (const time_t *timep);

static ACAI::ClientString commonTimeImage (GetBrokenDownTime break_time_r,
                                           const ACAI::ClientTimeStamp& ts,
                                           const int precision)
{
   ACAI::ClientString result;

   // Convert EPICS time to system time.
   //
   int nanoSec;
   const time_t utc = ACAI::utcTimeOf (ts, &nanoSec);

   // Form broken-down time bt
   //
   struct tm bt;

   // Make maybe uninitialised warnings go away.
   //
   bt.tm_year = bt.tm_mon = bt.tm_mday = bt.tm_hour = bt.tm_min = bt.tm_sec = 0;
   {
      struct tm *btPtr;
      btPtr = break_time_r (&utc);
      if (btPtr) {  // check success
         bt = *btPtr;
      }
   }

   // In broken-down time, tm_year is the number of years since 1900,
   // and January is month 0.
   //
   char text [40];
   snprintf (text, sizeof (text), "%04d-%02d-%02d %02d:%02d:%02d",
             1900 + bt.tm_year, 1 + bt.tm_mon, bt.tm_mday,
             bt.tm_hour, bt.tm_min, bt.tm_sec);

   result = text;

   // Add precision if required.
   //
   if (precision > 0) {
      static const int scale [10] = {
         1000000000, 100000000, 10000000, 1000000,
         100000, 10000, 1000, 100, 10, 1
      };

      char format[8];
      char fraction[16];

      const int p = MIN (precision, 9);
      snprintf (format, sizeof (format), ".%%0%dd", p);
      const int f =  nanoSec / scale[p];
      snprintf (fraction, sizeof (fraction), format, f);
      result.append (fraction);
   }

   return  result;
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::utcTimeImage (const ACAI::ClientTimeStamp& ts,
                                       const int precision)
{
   return commonTimeImage (gmtime, ts, precision);
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::localTimeImage (const ACAI::ClientTimeStamp& ts,
                                         const int precision)
{
   return commonTimeImage (localtime, ts, precision);
}

//------------------------------------------------------------------------------
//
ACAI_SHARED_FUNC
ACAI::ClientString ACAI::clientFieldTypeImage (const ACAI::ClientFieldType cft)
{
   ClientString result;

   // From base/modules/ca/src/client/access.cpp
   //
   switch (cft) {
      case ACAI::ClientFieldSTRING:    result = "DBF_STRING";    break;
      case ACAI::ClientFieldSHORT:     result = "DBF_SHORT";     break;
      case ACAI::ClientFieldFLOAT:     result = "DBF_FLOAT";     break;
      case ACAI::ClientFieldENUM:      result = "DBF_ENUM";      break;
      case ACAI::ClientFieldCHAR:      result = "DBF_CHAR";      break;
      case ACAI::ClientFieldLONG:      result = "DBF_LONG";      break;
      case ACAI::ClientFieldDOUBLE:    result = "DBF_DOUBLE";    break;
      case ACAI::ClientFieldNO_ACCESS: result = "DBF_NO_ACCESS"; break;
      case ACAI::ClientFieldDefault:   result = "Default";       break;
      default:
         csnprintf (result, 40, "Unknown field type (%d)", (int) cft);
         break;
   }
   return  result;
}

// end
