#include "Consumer.h"  


int main_Consumer(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  

	Broker broker("localhost:15555");

	Consumer c(&broker, "MyTopic"); 
	c.messageHander = [](Message* msg, MqClient* client, void* ctx) {
		msg->print();
		delete msg;
	};  
	c.start();  


	broker.join(); 
	return 0;
}