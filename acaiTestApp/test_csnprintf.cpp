// test_csnprintf.cpp
//

#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <acai_client_types.h>
#include <acai_version.h>
#include <string.h>

//------------------------------------------------------------------------------
// anonymise size value to avoid the pesky warning.
// The compiler, well g++ at least, is being too clever for own good.
//
static size_t anon (size_t n)
{
   long temp = (long)(&anon);
   temp = temp % 7;
   if ((temp & 1) == 0) temp = 1;  // is now def none zero
   return (temp * n) / temp;
}

//------------------------------------------------------------------------------
//
int main () {
   std::cout << "test csnprintf functions ("
             << ACAI_VERSION_STRING << ")"
             << std::endl << std::endl;

   ACAI::ClientString dest;
   ACAI::ClientString target;
   char buffer [512];
   int reqLen;
   const size_t n = anon(20);

   reqLen = ACAI::csnprintf (dest, n, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "req len " << reqLen << "  actual len " << dest.length() << std::endl;

   reqLen = snprintf (buffer, n, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "req len " << reqLen << "  actual len " << strnlen (buffer, 512) << std::endl;

   target = ACAI::csnprintf (n, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "target: " << target << "  target len " <<  target.length() << std::endl;
   std::cout << std::endl;

   dest = "1234567890123456";
   for (int j = 0; j < 12; j++) {
      const char* cdest;
      cdest = dest.c_str();
      reqLen = ACAI::csnprintf  (dest, j < 8 ? 512 : 800, "%s%s", cdest, cdest);
      std::cout << "req len " << std::setw (4) << reqLen
                << "  actual len " << std::setw (4) << dest.length()
                << std::endl;
   }
   std::cout << std::endl;

   std::cout << "test csnprintf complete" << std::endl;;
   return 0;
}

// end
