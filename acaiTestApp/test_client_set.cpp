// test_client_set.cpp
//

#include <iostream>
#include <acai_client_types.h>
#include <acai_client.h>
#include <acai_client_set.h>
#include <acai_version.h>


class TestClient : public ACAI::Client {
public:
   explicit TestClient (const ACAI::ClientString& pvName) :
      ACAI::Client (pvName)
   {
      std::cout << "constucted test client " << pvName << "\n";
   }

   ~TestClient ()
   {
      std::cout << "destructed test client " << this->pvName() << " \n";
   }
};

//------------------------------------------------------------------------------
//
static void dump (ACAI::Client* client, void* context)
{
   const char* message =  context ? (const char *) context : "nil";
   std::cout << "dump client (" << message << ") " << client->pvName() << "\n";
}

#define DUMP_SET(sn,expect) {                                         \
   std::cout << #sn << " iteration - expect " << expect << "\n";      \
   sn->iterateChannels (dump, (char*)(#sn));                          \
   std::cout << "count:    " << sn->count () << "\n";                 \
   std::cout << "contains: " << (sn->contains (t1) ? 1 : 0);          \
   std::cout << ", "         << (sn->contains (t2) ? 2 : 0);          \
   std::cout << ", "         << (sn->contains (t3) ? 3 : 0);          \
   std::cout << ", "         << (sn->contains (t4) ? 4 : 0);          \
   std::cout << ", "         << (sn->contains (t5) ? 5 : 0);          \
   std::cout << ", "         << (sn->contains (t6) ? 6 : 0);          \
   std::cout << "\n\n";                                               \
}


//------------------------------------------------------------------------------
//
int main () {
   std::cout << "test client set starting ("
             << ACAI_VERSION_STRING << ")\n\n";

   TestClient* t1 = new TestClient ("T1");
   TestClient* t2 = new TestClient ("T2");
   TestClient* t3 = new TestClient ("T3");
   TestClient* t4 = new TestClient ("T4");
   TestClient* t5 = new TestClient ("T5");
   TestClient* t6 = new TestClient ("T6");
   std::cout << "\n";

   ACAI::Client_Set* s1 = new ACAI::Client_Set ();
   ACAI::Client_Set* s2 = new ACAI::Client_Set (true);

   s1->insert (t1);
   s1->insert (t2);
   s1->insert (t3);
   s1->insert (t3);
   s1->insert (t3);

   s2->insert (t3);
   s2->insert (t4);
   s2->insert (t5);
   s2->insert (t6);

   DUMP_SET (s1, "T1,T2,T3");
   s1->remove (t2);
   DUMP_SET (s1, "T1,T3");

   DUMP_SET (s2, "T3,T4,T5,T6");
   s2->insertAllClients (s1);

   DUMP_SET (s1, "T1,T3");
   DUMP_SET (s2, "T1,T3,T4,T5,T6");

   std::cout << "clear set1\n";
   s1->clear();
   DUMP_SET (s1, "none");

   std::cout << "deleting set 1\n";
   delete s1;
   std::cout << "set 1 deleted\n";

   std::cout << "\ndeleting set 2\n";
   delete s2;
   std::cout << "set 2 delete\n";

   std::cout << "\ntest client set complete\n";
   return 0;
}

// end
