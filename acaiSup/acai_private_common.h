/* acai_private_common.h
 *
 * This file is part of the ACAI library. It provides utilities considered
 * private to the library. While this might get installed into <top>/include,
 * and hence visible to ACAI library users, there is no guarentee re it's
 * continued existance nor it's API backward compatibilty between releases,
 * even patch releases. Use at own risk.
 *
 * SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 */

#ifndef ACAI_PRIVATE_COMMON_H_
#define ACAI_PRIVATE_COMMON_H_

#include <iostream>
#include <exception>

// Useful type neutral numerical macro fuctions.
//
#define ABS(a)             ((a) >= 0  ? (a) : -(a))
#define MIN(a, b)          ((a) <= (b) ? (a) : (b))
#define MAX(a, b)          ((a) >= (b) ? (a) : (b))
#define LIMIT(x,low,high)  (MAX(low, MIN(x, high)))

// Calculates number of items in an array
//
#define ARRAY_LENGTH(xx) ( sizeof (xx) / sizeof (xx [0]) )

// Iterate over std::set or std::list (infact any std container)
//
#define ACAI_ITERATE(ContainerType, container, item)               \
   for (ContainerType::iterator item = container.begin ();         \
        item != container.end (); ++item)


// Catch exeptions used (initially) in the callback functions
//
#define ACAI_CATCH_EXCEPTION                                       \
catch (const std::exception& e) {                                  \
   std::cerr << __FUNCTION__ << " ("<< this->pvName() << "): "     \
             << "standard exception: '" << e.what() << "'\n";      \
}                                                                  \
catch (...) {                                                      \
   std::cerr << __FUNCTION__ << " ("<< this->pvName() << "): "     \
             << "unknown exception.\n";                            \
}


// Compiler specific code.
//
#if defined (_MSC_VER)
// Code specific to Visual Studio compiler
#  if (_MSC_VER < 1900)
// snprintf does not exist for earlier versions of MSVC, however _snprintf
// exists and will do.
#    define snprintf   _snprintf
#  endif
#endif

#endif   // ACAI_PRIVATE_COMMON_H_
