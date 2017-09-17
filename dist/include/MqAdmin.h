#ifndef __ZBUS_MQ_ADMIN_H__
#define __ZBUS_MQ_ADMIN_H__  
 
#include "MqClient.h"
#include "Broker.h"   
 
namespace zbus {

	class ZBUS_API MqAdmin { 
	public:
		std::string token;

		MqAdmin(Broker* broker);
		virtual ~MqAdmin();

		std::vector<TrackerInfo> queryTracker(int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ServerInfo> queryServer(int timeout = 3000, ServerSelector selector = NULL);
		std::vector<TopicInfo> queryTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ConsumeGroupInfo> queryGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<TopicInfo> declareTopic(std::string topic, int topicMask = -1, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ConsumeGroupInfo> declareGroup(std::string topic, ConsumeGroup& group, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ConsumeGroupInfo> declareGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ErrorInfo> removeTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL); 
		std::vector<ErrorInfo> removeGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL);
		std::vector<ErrorInfo> emptyTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL); 
		std::vector<ErrorInfo> emptyGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL);
	
	protected:
		Broker* broker;
		ServerSelector adminSelector;
	};

}//namespace
#endif