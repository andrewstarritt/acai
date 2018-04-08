/* acai_version.cpp
 *
 * This file is part of the ACAI library.
 *
 * Copyright (C) 2013-2018  Andrew C. Starritt
 *
 * The ACAI library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * You can also redistribute the ACAI library and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version when this library is disributed with and as part of the
 * EPICS QT Framework (https://github.com/qtepics).
 *
 * The ACAI library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * the Lesser GNU General Public License along with the ACAI library.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact details:
 * starritt@netspace.net.au
 * PO Box 3118, Prahran East, Victoria 3181, Australia.
 *
 */

#include <acai_version.h>

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
