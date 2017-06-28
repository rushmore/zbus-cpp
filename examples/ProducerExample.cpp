#include "Producer.h"  


int main_Producer(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO); 
	Logger* log = Logger::getLogger(); 


	Broker broker("localhost:15555");
	Producer p(&broker);
	std::vector<ServerInfo> res = p.queryServer();

	string topic = "CPP_Topic";
	p.declareTopic(topic);  

	Message msg;
	msg.setTopic(topic);
	msg.setBody("From C++ 11");

	p.produce(msg);

	system("pause");
	return 0;
}