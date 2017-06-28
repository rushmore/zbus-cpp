#ifndef __ZBUS_MQ_ADMIN_H__
#define __ZBUS_MQ_ADMIN_H__  
 
#include "MqClient.h"
#include "Broker.h"   

#define _MQADMIN_BEGIN(T,cmd,selector) \
	Message msg;\
	msg.setCmd((cmd));\
	if (selector == NULL) {\
		selector = this->adminSelector;\
	}\
	std::vector<T> res;\
	vector<MqClientPool*> pools = broker->select(selector, msg);\
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
	vector<MqClientPool*> pools = broker->select(selector, msg);\
	for (MqClientPool* pool : pools) {\
		MqClient* client = NULL;\
		ErrorInfo t;\
		try {\
			client = pool->borrowClient();


class ZBUS_API MqAdmin {
protected: 
	Broker* broker; 
	ServerSelector adminSelector;
public: 
	std::string token; 

	MqAdmin(Broker* broker) : broker(broker){
		adminSelector = [] (BrokerRouteTable& routeTable, Message& msg) {
			std::vector<ServerAddress> res;
			for (auto& kv : routeTable.serverTable) {
				res.push_back(kv.second.serverAddress);
			}
			return res;
		}; 
	}
	virtual ~MqAdmin() { }   

	std::vector<TrackerInfo> queryTracker(int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(TrackerInfo, PROTOCOL_TRACKER, selector)
			t = client->queryTracker(timeout);
		_MQADMIN_END()
	}

	std::vector<ServerInfo> queryServer(int timeout=3000, ServerSelector selector=NULL) {
		_MQADMIN_BEGIN(ServerInfo, PROTOCOL_QUERY, selector)
			t = client->queryServer(timeout);
		_MQADMIN_END()
	}  

	std::vector<TopicInfo> queryTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(TopicInfo, PROTOCOL_QUERY, selector)
			t = client->queryTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> queryGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_QUERY, selector)
			t = client->queryGroup(topic, group, timeout);
		_MQADMIN_END()
	} 

	std::vector<TopicInfo> declareTopic(std::string topic, int topicMask=-1, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(TopicInfo, PROTOCOL_DECLARE, selector)
			t = client->declareTopic(topic, topicMask, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> declareGroup(std::string topic, ConsumeGroup& group, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_DECLARE, selector)
			t = client->declareGroup(topic, group, timeout);
		_MQADMIN_END()
	}

	std::vector<ConsumeGroupInfo> declareGroup(std::string topic, string group, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_BEGIN(ConsumeGroupInfo, PROTOCOL_DECLARE, selector)
			t = client->declareGroup(topic, group, timeout);
		_MQADMIN_END()
	}


	std::vector<ErrorInfo> removeTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_REMOVE, selector)
			client->removeTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> removeGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_REMOVE, selector)
			client->removeGroup(topic, group, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> emptyTopic(std::string topic, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_EMPTY, selector)
			client->emptyTopic(topic, timeout);
		_MQADMIN_END()
	}

	std::vector<ErrorInfo> emptyGroup(std::string topic, std::string group, int timeout = 3000, ServerSelector selector = NULL) {
		_MQADMIN_VOID_BEGIN(PROTOCOL_EMPTY, selector)
			client->emptyGroup(topic, group, timeout);
		_MQADMIN_END()
	}   
};
  
#endif