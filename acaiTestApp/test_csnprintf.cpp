// test_csnprintf.cpp
//

#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <acai_client_types.h>
#include <acai_version.h>
#include <string.h>

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

   reqLen = ACAI::csnprintf (dest, 20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "req len " << reqLen << "  actual len " << dest.length() << std::endl;

   reqLen = snprintf (buffer, 20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "req len " << reqLen << "  actual len " << strnlen (buffer, 512) << std::endl;

   target = ACAI::csnprintf (20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
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
