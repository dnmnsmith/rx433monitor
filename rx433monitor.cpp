#include <iostream>
#include <stdio.h>
#include <pigpio.h>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <utility>
#include <signal.h>
#include <exception>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>

#include "ClimeMetDecoder.h"

#include "eventNotifier.h"

ClimeMetDecoder cmd;

//int g_jsonPort =  7004;
//std::string g_serverName = "webpi";


class Notifier : public IResultNotifier
{

 public:
   Notifier( int jsonPort, const std::string &serverName ) : m_notifier( jsonPort, serverName ) { }
 
   virtual void notify( const std::string &code, const std::string &meas, const std::string &value )
   {
      m_notifier.notifySensor( code, meas, value );
   } 
   
  private:
   
   eventNotifier m_notifier;

};

void dataFunction(int gpio, int level, uint32_t tick)
{
 static bool infn = 0;
 static uint32_t prev = 0;
 
 if (infn)
    return;
    
 infn = 1;
 
 uint32_t duration = tick - prev;
 prev = tick; 
 
 if (cmd.nextPulse(duration)) 	
 { 
//   cmd.print("CMD"); 
   cmd.notify();
   cmd.resetDecoder(); 
 }

 infn=0;
}

bool g_bGoing = true;

void signalHandler( int sig )
{
 g_bGoing = false;
}

int main( int argc, char ** argv)
{

   try
  {
    signal(SIGINT, signalHandler);

    // initialize logging - this reads the file log.xml from the current directory
    log_init();

    // read the command line options

    // option -i <ip-address> defines the ip address of the interface, where the
    // server waits for connections. Default is empty, which tells the server to
    // listen on all local interfaces
    cxxtools::Arg<std::string> ip(argc, argv, 'i');

    // option -p <number> specifies the port, where jsonrpc requests are expected.
    // The default port is 7004 here.
    cxxtools::Arg<unsigned short> port(argc, argv, 'p', 7004);
    cxxtools::Arg<std::string> serverName(argc, argv, 's', "webpi");

    Notifier notifier( port, serverName );

    cmd.setResultNotifier( &notifier );
    
    if (gpioInitialise() < 0)
    {
      throw std::runtime_error("gpioInitialise failed" );
    }

    gpioSetMode(18, PI_INPUT);
  
    if (gpioSetAlertFunc(18, &dataFunction) != 0)
    {
      throw std::runtime_error("gpioInitialise failed" );
    }

    while (g_bGoing) 
      sleep(1);
   }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  
  gpioTerminate();

 return 0;
}