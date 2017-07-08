// test_abstract_user.cpp
//

#include <iostream>
#include <acai_client_types.h>
#include <acai_client.h>
#include <acai_abstract_client_user.h>
#include <epicsThread.h>

//==============================================================================
//
//==============================================================================
//
class ClientUser : public ACAI::Abstract_Client_User {
public:
   explicit ClientUser ();
   ~ClientUser ();

protected:
   void connectionUpdate (ACAI::Client* sender,
                          const bool isConnected);
};

//------------------------------------------------------------------------------
//
ClientUser::ClientUser () : ACAI::Abstract_Client_User ()
{
   std::cout << "constucted client user\n";
}

//------------------------------------------------------------------------------
//
ClientUser::~ClientUser ()
{
   std::cout << "destucted client user\n";
}


//------------------------------------------------------------------------------
//
void ClientUser::connectionUpdate (ACAI::Client* sender,
                                   const bool isConnected)
{
   std::cout << "client user.connectionUpdate "
             << sender->pvName()
             << (isConnected ? " connected" : " disconnected")
             << "\n";
}


//==============================================================================
//
//==============================================================================
//
class TestClient : public ACAI::Client {
public:
   explicit TestClient (const ACAI::ClientString& pvName);
   ~TestClient ();

protected:
   void connectionUpdate (const bool isConnected);

};

//------------------------------------------------------------------------------
//
TestClient::TestClient (const ACAI::ClientString& pvName) :
   ACAI::Client (pvName)
{
   std::cout << "constucted test client " << pvName << "\n";
}

//------------------------------------------------------------------------------
//
TestClient::~TestClient ()
{
   std::cout << "destructed test client " << this->pvName() << " \n";
}

//------------------------------------------------------------------------------
//
void TestClient::connectionUpdate (const bool isConnected)
{
   std::cout << "test client connectionUpdate "
             << this->pvName()
             << (isConnected ? " connected" : " disconnected")
             << "\n";
}


//==============================================================================
//
int main () {
   std::cout << "test abstract client user starting ("
             << ACAI_VERSION_STRING << ")\n\n";

   ACAI::Client::initialise ();

   ClientUser* user = new ClientUser ();

   TestClient* t1 = new TestClient ("T1");
   TestClient* t2 = new TestClient ("T2");
   TestClient* t3 = new TestClient ("T3");
   TestClient* t4 = new TestClient ("T4");

   user->registerClient (t1);
   user->registerClient (t2);
   user->registerClient (t3);
   std::cout << "\n";

   std::cout << "open registered clients\n";
   user->openRegisteredChannels();
   std::cout << "registered clients opened\n";

   std::cout << "open T4 client\n";
   t4->openChannel();
   std::cout << "T4 client opened\n";

   bool ok = user->waitAllRegisteredChannelsReady (2.0, 0.1);
   std::cout << "all channels open " << (ok ? "yes" : "no") <<  "\n";

   for (int t = 0; t < 500; t++) {
      epicsThreadSleep (0.02);   // 20mSec
      ACAI::Client::poll ();     // call back function invoked from here
   }

   std::cout << "close registered clients\n";
   user->closeRegisteredChannels();

   delete user;

   ACAI::Client::finalise();
   std::cout << "\ntest abstract client user complete\n";
   return 0;
}

// end
