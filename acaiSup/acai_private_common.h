/* acai_private_common.h
 *
 * This file is part of the ACAI library. It provides utilities considered
 * private to the library. While installed into <top>/include, and hence
 * visible to ACAI library users, there is no guarentee re API backward
 * compatibilty between releases, even patch releases.
 *
 * Copyright (C) 2015-2020  Andrew C. Starritt
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
 * andrew.starritt@gmail.com
 * PO Box 3118, Prahran East, Victoria 3181, Australia.
 *
 */

#ifndef ACAI__PRIVATE_COMMON_H_
#define ACAI__PRIVATE_COMMON_H_

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

#endif   // ACAI__PRIVATE_COMMON_H_
