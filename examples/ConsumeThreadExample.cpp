#include "Consumer.h"  


int main_ConsumeThread(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  

	MqClientPool pool("localhost:15555");

	ConsumeThread ct(&pool);
	ct.topic = "MyTopic"; 
	ct.connectionCount = 1;
	
	ct.messageHander = [](Message* msg, MqClient* client, void* ctx) {
		msg->print();
		delete msg;
	};
	ct.start();

	system("pause");
	return 0;
}