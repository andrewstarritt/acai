// test_csnprintf.cpp
//

#include <iostream>
#include <acai_client_types.h>
#include <acai_version.h>
#include <string.h>

//------------------------------------------------------------------------------
//
int main () {
   std::cout << "test csnprintf functions ("
             << ACAI_VERSION_STRING << ")\n\n";


   ACAI::ClientString dest;
   char buffer [512];
   int reqLen;

   reqLen = ACAI::csnprintf (dest, 20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << reqLen << "  dest: " << dest <<"\n";

   reqLen = snprintf (buffer, 20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << reqLen << "  dest: " << buffer <<"\n";

   ACAI::ClientString target;
   target = ACAI::csnprintf (20, "0123456789%s0123456789%s0123456789", "ABCDE", "FGHIJ");
   std::cout << "  target: " << target <<"\n";

   std::cout << "\ntest csnprintf complete\n";
   return 0;
}

// end
