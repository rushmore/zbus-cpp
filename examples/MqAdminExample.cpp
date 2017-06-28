#include "MqAdmin.h"  


int main_MqAdmin(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO); 
	Logger* log = Logger::getLogger(); 


	Broker broker("localhost:15555");
	MqAdmin admin(&broker);
	std::vector<ServerInfo> res = admin.queryServer();

	string topic = "CPP_Topic";
	admin.declareTopic(topic); 
	admin.declareGroup(topic, "MyCpp");
	admin.removeTopic(topic);

	system("pause");
	return 0;
}