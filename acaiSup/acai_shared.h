/* acai_shared.h
 *
 * This file is part of the ACAI library.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
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
