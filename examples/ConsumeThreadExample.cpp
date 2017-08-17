#include "Consumer.h"  
using namespace zbus;
using namespace std;

int main_ConsumeThread(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  

	MqClientPool pool("localhost:15555");

	ConsumeThread ct(&pool);
	ct.topic = "MyTopic"; 
	ct.connectionCount = 1;
	
	ct.messageHander = [](Message* msg, MqClient* client) {
		msg->print();
		delete msg;
	};
	ct.start();

	ct.join();
	return 0;
}