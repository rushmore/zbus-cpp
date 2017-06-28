#include "MqClient.h"  



int main_MqClient(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO); 
	Logger* log = Logger::getLogger();

	MqClient client("localhost:15555");
	client.connect();
	 
	TrackerInfo info = client.queryTracker();
	log->info("%s", info.serverAddress.address.c_str());

	string topic = "CPP_Topic";
	client.declareTopic(topic);

	TopicInfo topicInfo = client.queryTopic(topic); 
	log->info("%s", topicInfo.topicName.c_str());

	ConsumeGroupInfo groupInfo = client.queryGroup(topic, "CPP_Topic");
	log->info("%s", groupInfo.groupName.c_str()); 
	 
	ConsumeGroup group;
	group.groupName = "MyCpp";
	group.filter = "abc.*";

	client.declareGroup(topic, group);

	Message msg;
	msg.setTopic(topic);
	msg.setBody("From C++ 11, cool man");

	client.produce(msg);

	Message* res = client.consume(topic);
	res->print();
	delete res;


	client.removeGroup(topic, "MyCpp");

	client.removeTopic(topic); 


	system("pause");
	return 0;
}