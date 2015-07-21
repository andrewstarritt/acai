/* $File: //depot/sw/epics/acai/acaiSup/acai_client_types.h $
 * $Revision: #10 $
 * $DateTime: 2015/06/22 20:47:28 $
 * $Author: andrew $
 *
 * This file is part of the ACAI library.
 *
 * Copyright (C) 2013,2014,2015  Andrew C. Starritt
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

#ifndef ACAI__CLIENT_TYPES_H
#define ACAI__CLIENT_TYPES_H

#include <string>
#include <vector>

/// \brief
/// All ACAI classes, typedefs, functons and enumerations are declared within
/// the ACAI namespace.
///
namespace ACAI {

/// Defines common ACAI macros, types and pseudo EPICS CA types.
///
#define ACAI_VERSION_STRING     "ACAI 1.1.1"

// Place holder to deal with shared stuff.
// Not really important for Linux.
//
#if defined(ACAI_LIBRARY)
#define ACAI_SHARED_CLASS
#else
#define ACAI_SHARED_CLASS
#endif


/// MUST be consistent with PVNAME_STRINGSZ out of dbDefs.h
/// Includes the nil terminator
///
#define ACAI_MAX_PVNAME_LENGTH    61


/// Controls how channel operates when channel opened.
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


//------------------------------------------------------------------------------
// Pseudo CA types.
// Allows clients that build against this library to need only include
// library header files without the need to include EPICS header files.
//
// These types effectively replicate the EPICS types. Must be kept is step.
// But these types really really stable so this is not a hassle.
//
// MUST be consistent with alarm.h
/// Extends stardard EPICS severity to include a disconnected state.
typedef enum {
   ClientSevNone  = 0,
   ClientSevMinor = 1,
   ClientSevMajor = 2,
   ClientSevInvalid = 3,
   ClientDisconnected = 4,
   CLIENT_ALARM_NSEV
} ClientAlarmSeverity;


/// Severity status - essentially a copy of the epicsAlarmCondition.
typedef enum {
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
} ClientAlarmCondition;


/// Time stamp structure - essentially a copy of epicsTimeStamp.
typedef struct {
   ACAI::ClientUInt32 secPastEpoch;     ///< seconds since 0000 Jan 1, 1990  UTC.
   ACAI::ClientUInt32 nsec;             ///< nanoseconds within second
} ClientTimeStamp;


// MUST be consistent with db_access.h
/// Field type. Essentially a copy of db_access.h with addtion of a default type used for requests only.
typedef enum {
   ClientFieldSTRING    = 0,
   ClientFieldSHORT     = 1,
   ClientFieldFLOAT     = 2,
   ClientFieldENUM      = 3,
   ClientFieldCHAR      = 4,
   ClientFieldLONG      = 5,
   ClientFieldDOUBLE    = 6,
   ClientFieldNO_ACCESS = 7,
   // Additional pseudo field type used for requests.
   // Mixing control and data here, but this is pragmatic.
   ClientFieldDefault   = 8
} ClientFieldType;


//------------------------------------------------------------------------------
/// This function provides a textual/displayable image for the field type.
///
ACAI::ClientString clientFieldTypeImage (const ACAI::ClientFieldType clientFieldType);

//------------------------------------------------------------------------------
// ClientString utility functions.
//
/// Like snprintf except targets a ACAI::ClientString.
///
/// The size_t size paramter is used for an internal buffer (note: this is on the stack)
/// and will constrains the final string size.
///
int csnprintf (ACAI::ClientString& target, size_t size, const char* format, ...);

/// This creates and returns a ACAI::ClientString
///
/// The size_t size paramter is used for an internal buffer (note: this is on the stack)
/// and will constrains the final string size.
///
ACAI::ClientString csnprintf (size_t size, const char* format, ...);

}

#endif // ACAI__CLIENT_TYPES_H