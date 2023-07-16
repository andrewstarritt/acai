/* acai_client_types.h
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

#ifndef ACAI_CLIENT_TYPES_H
#define ACAI_CLIENT_TYPES_H

#include <time.h>
#include <string>
#include <vector>
#include <acai_shared.h>

/// \brief
/// All ACAI classes, typedefs, functons and enumerations are declared within
/// the ACAI namespace.
///
namespace ACAI {

// Defines common ACAI macros, types and pseudo EPICS CA types.
//

// Note: Shared related macros relocated to acai_shared.h
// Note: Version related macros relocated to acai_version.h



/// \brief Controls how channel operates when the channel is opened. Subscribe is the default mode.
///
enum ReadModes {
   NoRead,             ///< just connects
   SingleRead,         ///< single one-off read only
   Subscribe           ///< read plus subscription - default mode.
};


//------------------------------------------------------------------------------
// Defines client string, integer and floating point data types.
// Uses basic types for scalars and vectors (arrays) which are consistant
// with the Channel Access types.
//
// Architecture Independent Data Types
// (so far, this is sufficient for all archs we have ported to)
// Cribbed from epicsTypes.h
//
#if __STDC_VERSION__ >= 199901L

/// Provides the type used to read/write channel data as an integer value.
typedef int32_t        ClientInteger;

/// Provides the type used represent an unsigned integer value.
typedef uint32_t       ClientUInt32;

#else

/// Provides the type used to read/write channel data as an integer value.
typedef int            ClientInteger;

/// Provides the type used represent an unsigned integer value.
typedef unsigned int   ClientUInt32;

#endif

/// Provides the type used to read/write channel data as a floating value.
typedef double         ClientFloating;

/// Provides the type used to read/write channel data as a string value.
typedef std::string    ClientString;

// Used for array data.
//
/// Provides the array type used to read/write channel data as integer values.
typedef std::vector<ClientInteger>    ClientIntegerArray;

/// Provides the array type used to read/write channel data as floating values.
typedef std::vector<ClientFloating>   ClientFloatingArray;

/// Provides the array type used to read/write channel data as string values.
typedef std::vector<ClientString>     ClientStringArray;

/// Provides the array type used to read/write channel data as boolean values.
typedef std::vector<bool>             ClientBooleanArray;


//------------------------------------------------------------------------------
// Pseudo CA macros/types.
// Allows clients that build against this library to need only include
// library header files without the need to include EPICS header files.
//
// These types effectively replicate the EPICS Channel Access types. Must be kept is step.
// But as the EPICS Channel Access types are really really stable so this is not a hassle.
//

// MUST be kept consistent with alarm.h
/// \brief Extends standard EPICS severity to include a disconnected state.
//
enum ClientAlarmSeverity {
   ClientSevNone  = 0,
   ClientSevMinor = 1,
   ClientSevMajor = 2,
   ClientSevInvalid = 3,
   ClientDisconnected = 4,
   CLIENT_ALARM_NSEV
};

/// \brief Severity status - essentially a copy of the epicsAlarmCondition.
///
enum ClientAlarmCondition {
   ClientAlarmNone = 0,
   ClientAlarmRead,
   ClientAlarmWrite,
   ClientAlarmHiHi,
   ClientAlarmHigh,
   ClientAlarmLoLo,
   ClientAlarmLow,
   ClientAlarmState,
   ClientAlarmCos,
   ClientAlarmComm,
   ClientAlarmTimeout,
   ClientAlarmHwLimit,
   ClientAlarmCalc,
   ClientAlarmScan,
   ClientAlarmLink,
   ClientAlarmSoft,
   ClientAlarmBadSub,
   ClientAlarmUDF,
   ClientAlarmDisable,
   ClientAlarmSimm,
   ClientAlarmReadAccess,
   ClientAlarmWriteAccess,
   CLIENT_ALARM_NSTATUS
};


/// \brief Time stamp structure - essentially a copy of epicsTimeStamp.
///
struct ClientTimeStamp {
   ACAI::ClientUInt32 secPastEpoch;     ///< seconds since 0000 Jan 1, 1990  UTC.
   ACAI::ClientUInt32 nsec;             ///< nanoseconds within second
};


// MUST be kept consistent with db_access.h
/// \brief Field type. Essentially a copy of db_access.h with the addition of
/// a default type used for requests only.
///
enum ClientFieldType {
   ClientFieldSTRING    = 0,
   ClientFieldSHORT     = 1,
   ClientFieldFLOAT     = 2,
   ClientFieldENUM      = 3,
   ClientFieldCHAR      = 4,
   ClientFieldLONG      = 5,
   ClientFieldDOUBLE    = 6,
   ClientFieldNO_ACCESS = 7,
   // Additional pseudo field type used for requests.
   // Mixing control and data here, but it is pragmatic.
   ClientFieldDefault   = 8
};

/// \brief Controls event subscriptions - refer to caeventmask.h for details.
/// The default mode is Value | Alarm
// Keep consistant with caeventmask.h
//
enum EventMasks {
   EventNone = 0,           ///< No trigger. Included for completeness
   EventValue =    (1<<0),  ///< Trigger an event when a significant change in the channel's value (usually defined by MDEL)
   EventArchive =  (1<<1),  ///< Trigger an event when an archive significant change in the channel's value occurs (usually defined by ADEL)
   EventAlarm =    (1<<2),  ///< Trigger an event when the alarm state changes
   EventProperty = (1<<3)   ///< Trigger an event when a property change (control limit, graphical limit, status string, enum string ...) occurs.
};

//------------------------------------------------------------------------------
/// This function returns true if the specified alarm severity is one of no alarm,
/// minor alarm or major alarm otherwise false, i.e. when invalid or disconnected.
///
ACAI_SHARED_FUNC bool alarmSeverityIsValid (const ACAI::ClientAlarmSeverity severity);

//------------------------------------------------------------------------------
/// This function returns a textual/displayable form of the specified alarm severity.
///
ACAI_SHARED_FUNC ACAI::ClientString alarmSeverityImage (const ACAI::ClientAlarmSeverity severity);

//------------------------------------------------------------------------------
/// This function returns a textual/displayable form of the specified alarm status.
///
ACAI_SHARED_FUNC ACAI::ClientString alarmStatusImage (const ACAI::ClientAlarmCondition status);

//------------------------------------------------------------------------------
/// This function returns the time_t type of the given client time stamp.
/// This take into account the EPICS epoch offet.
///
ACAI_SHARED_FUNC time_t utcTimeOf (const ACAI::ClientTimeStamp& ts, int* nanoSecOut = NULL);

//------------------------------------------------------------------------------
/// This function returns a textual/displayable form of the time stamp.
/// Format is: "yyyy-mm-dd hh:nn:ss[.ffff]"
/// Without fractions, this is a suitable format for MySql.
///
ACAI_SHARED_FUNC ACAI::ClientString utcTimeImage (const ACAI::ClientTimeStamp& ts,
                                                  const int precision = 0);

//------------------------------------------------------------------------------
/// This function returns a textual/displayable form of the time stamp.
/// Format is: "yyyy-mm-dd hh:nn:ss[.ffff]"
/// As above, but local time.
///
ACAI_SHARED_FUNC ACAI::ClientString localTimeImage (const ACAI::ClientTimeStamp& ts,
                                                    const int precision = 0);

//------------------------------------------------------------------------------
/// This function provides a textual/displayable image for the field type.
///
ACAI_SHARED_FUNC ACAI::ClientString clientFieldTypeImage (const ACAI::ClientFieldType clientFieldType);

//------------------------------------------------------------------------------
// ClientString utility functions.
//
/// Like snprintf except targets an ACAI::ClientString.
///
/// The size_t size parameter is used to size internal buffer and will constrain
/// the final string size. The return value is thr required string size.
///
ACAI_SHARED_FUNC int csnprintf (ACAI::ClientString& target, size_t size, const char* format, ...);

/// This creates and returns an ACAI::ClientString
///
/// The size_t size parameter is used to size internal buffer and will constrain
/// the returned string size.
///
ACAI_SHARED_FUNC ACAI::ClientString csnprintf (size_t size, const char* format, ...);

/// Assign at most maxSize characters to ACAI::ClientString. This is useful
/// for 'full' fixed size string (e.g. an enumeration value) which does not
/// include a terminating null character. Cribbed from epicsQt.
/// Kind of like strncpy, however result.c_str() always is null terminated.
///
ACAI_SHARED_FUNC ACAI::ClientString limitedAssign (const char* source, const size_t maxSize);

} // ACAI namespace

#endif // ACAI_CLIENT_TYPES_H
