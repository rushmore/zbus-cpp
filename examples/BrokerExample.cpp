#include "Broker.h"  
#include <iostream>
using namespace zbus;
using namespace std;

int main_broker(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_DEBUG);  

	Broker* broker = new Broker("localhost:15555;localhost:15556");

	delete broker; 
	return 0;
}