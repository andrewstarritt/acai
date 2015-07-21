/* $File: //depot/sw/epics/acai/acaiSup/acai_client_types.cpp $
 * $Revision: #7 $
 * $DateTime: 2015/06/21 17:41:41 $
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

#include <stdio.h>
#include <stdarg.h>
#include <acai_client_types.h>

//------------------------------------------------------------------------------
//
int ACAI::csnprintf (ACAI::ClientString& target, size_t size, const char* format, ...)
{
   va_list args;
   char buffer [size + 1];
   int result;

   va_start (args, format);
   result = vsnprintf (buffer, sizeof (buffer), format, args);
   va_end (args);

   target = buffer;
   return result;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::csnprintf (size_t size, const char* format, ...)
{
   va_list args;
   char buffer [size + 1];
   int len;

   va_start (args, format);
   len = vsnprintf (buffer, sizeof (buffer), format, args);
   va_end (args);

   return buffer;
}

//------------------------------------------------------------------------------
//
ACAI::ClientString ACAI::clientFieldTypeImage (const ACAI::ClientFieldType cft)
{
   ClientString result;

   switch (cft) {
      case ACAI::ClientFieldSTRING:    result = "STRING";    break;
      case ACAI::ClientFieldSHORT:     result = "SHORT";     break;
      case ACAI::ClientFieldFLOAT:     result = "FLOAT";     break;
      case ACAI::ClientFieldENUM:      result = "ENUM";      break;
      case ACAI::ClientFieldCHAR:      result = "CHAR";      break;
      case ACAI::ClientFieldLONG:      result = "LONG";      break;
      case ACAI::ClientFieldDOUBLE:    result = "DOUBLE";    break;
      case ACAI::ClientFieldNO_ACCESS: result = "NO_ACCESS"; break;
      case ACAI::ClientFieldDefault:   result = "Default";   break;
      default:
         csnprintf (result, 40, "Unknown field type (%d)", (int) cft);
         break;
   }
   return  result;
}

// end
