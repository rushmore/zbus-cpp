#include "Broker.h"  


int main_Broker(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_DEBUG); 
	Logger* log = Logger::getLogger();  

	Broker broker("localhost:15555");
	 

	broker.join(); //wait for underlying threads
	return 0;
}