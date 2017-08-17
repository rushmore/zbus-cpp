#include "Consumer.h"  
#include <iostream>
using namespace zbus;
using namespace std;

int main_consumer(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  

	for (int i = 0; i < 100000000; i++) {
		Message msg;
		msg.setCmd("produce");
		ByteBuffer buf;
		msg.encode(buf);
		buf.flip();
		Message* m = Message::decode(buf);
		if (m != NULL) {
			delete m;
		}
		if (i % 10000 == 0) {
			std::this_thread::sleep_for(1s);
			cout << i << endl;
		} 
	} 

	Broker broker("localhost:15555");

	Consumer c(&broker, "MyTopic"); 
	c.messageHander = [](Message* msg, MqClient* client) {
		msg->print();
		delete msg;
	};  
	c.start();  


	broker.join(); 
	return 0;
}