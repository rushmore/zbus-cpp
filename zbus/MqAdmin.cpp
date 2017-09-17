#include "MqAdmin.h"


#define _MQADMIN_BEGIN(T,cmd,selector) \
	Message msg;\
	msg.setCmd((cmd));\
	if (selector == NULL) {\
		selector = this->adminSelector;\
	}\
	std::vector<T> res;\
	std::vector<MqClientPool*> pools = broker->select(selector, msg);\
	for (MqClientPool* pool : pools) {\
		MqClient* client = NULL;\
		T t;\
		try {\
			client = pool->borrowClient();

#define _MQADMIN_END() \
        }\
		catch (MqException& e) {\
			t.setError(e);\
		}\
		pool->returnClient(client);\
		res.push_back(t);\
	}\
	return res;

#define _MQADMIN_VOID_BEGIN(cmd,selector) \
	Message msg;\
	msg.setCmd((cmd));\
	if (selector == NULL) {\
		selector = this->adminSelector;\
	}\
	std::vector<ErrorInfo> res;\
	std::vector<MqClientPool*> pools = broker->select(selector, msg);\
	for (MqClientPool* pool : pools) {\
		MqClient* client = NULL;\
		ErrorInfo t;\
		try {\
			client = pool->borrowClient();

namespace zbus {
	 
	MqAdmin::MqAdmin(Broker* broker) : broker(broker) {
		adminSelector = [](BrokerRouteTable& routeTable, Message& msg) {
			std::vector<ServerAddress> res;
			for (auto& kv : routeTable.getServerTable()) {
				res.push_back(kv.second.serverAddress);
			}
			return res;
		};
	}
	MqAdmin::~MqAdmin() { }

	std::vector<TrackerInfo> MqAdmin::queryTracker(int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(TrackerInfo, PROTOCOL_TRACKER, selector)
			t = client->queryTracker(timeout);
		_MQADMIN_END()
	}

	std::vector<ServerInfo> MqAdmin::queryServer(int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(ServerInfo, PROTOCOL_QUERY, selector)
			t = client->queryServer(timeout);
		_MQADMIN_END()
	}

	std::vector<TopicInfo> MqAdmin::queryTopic(std::string topic, int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(TopicInfo, PROTOCOL_QUERY, selector)
			t = client->queryTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> MqAdmin::queryGroup(std::string topic, std::string group, 
		int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_QUERY, selector)
			t = client->queryGroup(topic, group, timeout);
		_MQADMIN_END()
	}

	std::vector<TopicInfo> MqAdmin::declareTopic(std::string topic, 
		int topicMask, int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(TopicInfo, PROTOCOL_DECLARE, selector)
			t = client->declareTopic(topic, topicMask, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> MqAdmin::declareGroup(std::string topic, ConsumeGroup& group, 
		int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_DECLARE, selector)
			t = client->declareGroup(topic, group, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> MqAdmin::declareGroup(std::string topic, std::string group, 
		int timeout, ServerSelector selector) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_DECLARE, selector)
			t = client->declareGroup(topic, group, timeout);
		_MQADMIN_END()
	} 

	std::vector<ErrorInfo> MqAdmin::removeTopic(std::string topic, 
		int timeout, ServerSelector selector) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_REMOVE, selector)
			client->removeTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> MqAdmin::removeGroup(std::string topic, std::string group, 
		int timeout, ServerSelector selector) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_REMOVE, selector)
			client->removeGroup(topic, group, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> MqAdmin::emptyTopic(std::string topic, 
		int timeout, ServerSelector selector) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_EMPTY, selector)
			client->emptyTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> MqAdmin::emptyGroup(std::string topic, std::string group, 
		int timeout, ServerSelector selector) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_EMPTY, selector)
			client->emptyGroup(topic, group, timeout);
		_MQADMIN_END()
	} 

}//namespace