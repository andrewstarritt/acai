/* acai_shared.h
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

#ifndef ACAI_SHARED_H
#define ACAI_SHARED_H

// Deal with shared stuff.
// Not really important for Linux but Windows needs help.
//
#if defined(_WIN32)
#   if defined(BUILDING_ACAI_LIBRARY)
#      define ACAI_SHARED_CLASS __declspec(dllexport)
#      define ACAI_SHARED_FUNC  __declspec(dllexport)
#   else
#      define ACAI_SHARED_CLASS __declspec(dllimport)
#      define ACAI_SHARED_FUNC  __declspec(dllimport)
#   endif
#elif __GNUC__ >= 4
#   define ACAI_SHARED_CLASS    __attribute__ ((visibility("default")))
#   define ACAI_SHARED_FUNC     __attribute__ ((visibility("default")))
#else
#warning "Unknown compiler environment - proceeding cautiously"
#endif

#endif // ACAI_SHARED_H
