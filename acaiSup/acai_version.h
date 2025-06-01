/* acai_client_types.h
 *
 * This file is part of the ACAI library.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 */

#ifndef ACAI_VERSION_H
#define ACAI_VERSION_H

#include "acai_client_types.h"
#include "acai_shared.h"

// Defines the major version number, this increments when there is a major paradigm shift.
//
#define ACAI_MAJOR              1

// Defines the minor version number, this increments when there is a non compatibile API change.
//
#define ACAI_MINOR              8

// Defines the patch version number, this increments for bug fixes and/or
// backward compatible API enhancements.
//
#define ACAI_PATCH              1

// NOTE: Don't forget to update documentation/acai.cfg and acaiSup/Makefile

// Integer and string versions
//
// The integer version is (major << 16) + (minor << 8) + patch, and this macro
// is used to constuct an integer version number.
//
#define ACAI_INT_VERSION(major, minor, patch) (((major)<<16)|((minor)<<8)|(patch))

// ACAI_VERSION is the actual version of this version of ACAI.
// It can be used like this to perform version specific checking
// #if (ACAI_VERSION >= ACAI_INT_VERSION(1, 8, 1))
//
#define ACAI_VERSION            ACAI_INT_VERSION (ACAI_MAJOR, ACAI_MINOR, ACAI_PATCH)

// Allows artefacts to be convert to a string
//
#define ACAI_STRINGIFY_INNER(s) #s
#define ACAI_STRINGIFY(s)       ACAI_STRINGIFY_INNER(s)

// Define the string version of ACAI, e.g. "ACAI 1.8.1"
//
#define ACAI_VERSION_STRING     "ACAI " ACAI_STRINGIFY(ACAI_MAJOR) \
                                "."     ACAI_STRINGIFY(ACAI_MINOR) \
                                "."     ACAI_STRINGIFY(ACAI_PATCH)

namespace ACAI {

/// Returns runtime integer version, as opposed to compile time header version.
///
ACAI_SHARED_FUNC int version ();

/// Returns runtime string version, as opposed to compile time header version.
///
ACAI_SHARED_FUNC ACAI::ClientString versionString ();

} // ACAI namespace


#endif // ACAI_VERSION_H
