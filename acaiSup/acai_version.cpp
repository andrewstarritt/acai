/* acai_version.cpp
 *
 * This file is part of the ACAI library.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 */

#include "acai_version.h"

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
