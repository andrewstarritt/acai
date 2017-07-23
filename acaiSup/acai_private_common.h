/* acai_private_common.h
 *
 * This file is part of the ACAI library. It provides utilities considered
 * private tto the library.
 *
 * Copyright (C) 2015,2017  Andrew C. Starritt
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
#define ITERATE(ContainerType, container, item)                    \
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


#endif   // ACAI__PRIVATE_COMMON_H_
