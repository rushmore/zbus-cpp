#ifndef __ZBUS_MQ_CLIENT_H__
#define __ZBUS_MQ_CLIENT_H__  
 
#include "MessageClient.h"
#include "Kit.h"  

#include <queue>
#include <mutex>
#include <condition_variable>

 
class ZBUS_API ConsumeGroup {
public:
	std::string groupName;
	std::string filter;          //filter on message'tag
	int mask = -1;

	std::string startCopy;       //create group from another group 
	int64_t startOffset = -1;
	std::string startMsgId;      //create group start from offset, msgId to check valid
	int64_t startTime = -1;    //create group start from time 

	void load(Message& msg) {
		groupName = msg.getConsumeGroup();
		filter = msg.getGroupFilter();
		mask = msg.getGroupMask();
		startCopy = msg.getGroupStartCopy();
		startOffset = msg.getGroupStartOffset();
		startMsgId = msg.getGroupStartMsgId();
		startTime = msg.getGroupStartTime();
	}

	void writeTo(Message& msg) {
		msg.setConsumeGroup(groupName);
		msg.setGroupFilter(filter); 
		msg.setGroupMask(mask);
		msg.setGroupStartCopy(startCopy);
		msg.setGroupStartOffset(startOffset);
		msg.setGroupStartMsgId(startMsgId);
		msg.setGroupStartTime(startTime);
	}
};

class ZBUS_API MqClient : public MessageClient {

public:
	std::string token;

	MqClient(std::string address, bool sslEnabled = false, std::string sslCertFile = "") :
		MessageClient(address, sslEnabled, sslCertFile){

	}
	virtual ~MqClient() { }


	Message produce(Message& msg, int timeout = 3000) { 
		msg.setCmd(PROTOCOL_PRODUCE);
		if (msg.getToken() == "") {
			msg.setToken(token);
		} 
		Message* res = invoke(msg, timeout);
		Message ret(*res);
		delete res;
		return ret; 
	}

	Message* consume(string topic, string group="", int window=-1, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_CONSUME);
		msg.setTopic(topic);
		msg.setConsumeGroup(group);
		msg.setConsumeWindow(window);
		
		if (msg.getToken() == "") {
			msg.setToken(token);
		}
		return invoke(msg, timeout); 
	}



	TrackerInfo queryTracker(int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_TRACKER);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TrackerInfo info;
		parseTrackerInfo(info, *res);

		if (res) delete res; 
		return info;
	}

	ServerInfo queryServer(int timeout=3000) {
		Message msg;
		msg.setCmd(PROTOCOL_SERVER);
		msg.setToken(token); 
		Message* res = invoke(msg, timeout); 

		ServerInfo info;
		parseServerInfo(info, *res); 

		if (res) delete res; 
		return info;
	}  

	TopicInfo queryTopic(std::string topic, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_QUERY);
		msg.setTopic(topic);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TopicInfo info;
		parseTopicInfo(info, *res);

		if (res) delete res; 
		return info;
	}

	ConsumeGroupInfo queryGroup(std::string topic, std::string group, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_QUERY);
		msg.setTopic(topic);
		msg.setConsumeGroup(group);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		ConsumeGroupInfo info;
		parseConsumeGroupInfo(info, *res);

		if (res) delete res; 
		return info;
	} 

	TopicInfo declareTopic(std::string topic, int topicMask=-1, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_DECLARE);
		msg.setTopic(topic);
		msg.setTopicMask(topicMask);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TopicInfo info;
		parseTopicInfo(info, *res);

		if (res)  delete res; 
		return info;
	}

	ConsumeGroupInfo declareGroup(std::string topic, string group, int timeout = 3000) {
		ConsumeGroup consumeGroup;
		consumeGroup.groupName = group;
		return declareGroup(topic, consumeGroup, timeout);
	}

	ConsumeGroupInfo declareGroup(std::string topic, ConsumeGroup& group, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_DECLARE);
		msg.setTopic(topic); 
		msg.setToken(token);
		group.writeTo(msg);

		Message* res = invoke(msg, timeout);

		ConsumeGroupInfo info;
		parseConsumeGroupInfo(info, *res);

		if (res)  delete res; 

		return info;
	}

	void removeTopic(std::string topic, int timeout = 3000) {
		removeGroup(topic, "", timeout);
	}

	void removeGroup(std::string topic, std::string group, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_REMOVE);
		msg.setTopic(topic);
		msg.setConsumeGroup(group);
		msg.setToken(token); 

		Message* res = invoke(msg, timeout);

		if (res && res->status != "200") {
			MqException error(res->getBodyString());
			delete res;
			throw error;
		}

		if (res) delete res; 
	}

	void emptyTopic(std::string topic, int timeout = 3000) {
		emptyGroup(topic, "", timeout);
	}

	void emptyGroup(std::string topic, std::string group, int timeout = 3000) {
		Message msg;
		msg.setCmd(PROTOCOL_EMPTY);
		msg.setTopic(topic);
		msg.setConsumeGroup(group);
		msg.setToken(token);

		Message* res = invoke(msg, timeout);

		if (res && res->status != "200") {
			MqException error(res->getBodyString());
			delete res;
			throw error;
		}

		if (res) delete res;
	}  

	void route(Message& msg, int timeout = 3000) {
		msg.setCmd(PROTOCOL_ROUTE); 
		msg.setAck(false);
		if (msg.getToken() == "") {
			msg.setToken(token);
		} 
		if (msg.status != "") {
			msg.setOriginStatus(msg.status);
			msg.status = "";
		}
		send(msg, timeout);
	}
};
 
class ZBUS_API MqClientPool {
public:
	MqClientPool(std::string serverAddress, int maxSize = 32, bool sslEnabled = false, std::string sslCertFile = "") :
		maxSize(maxSize),
		serverAddress(serverAddress), sslEnabled(sslEnabled), sslCertFile(sslCertFile)
	{

	} 

	virtual ~MqClientPool() { 
		std::unique_lock<std::mutex> lock(mutex);
		while (!queue.empty()) {
			MqClient* client = queue.front(); 
			delete client;
			queue.pop();
		}
	}

	void returnClient(MqClient* value) {
		if (value == NULL) return;
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.push(value);
		}
		signal.notify_one();
	}

	MqClient* borrowClient() {
		std::unique_lock<std::mutex> lock(mutex);
		if (size<maxSize && queue.empty()) {
			MqClient* client = makeClient();
			queue.push(client);
			size++;
		}
		while (queue.empty()) {
			signal.wait(lock);
		}

		MqClient* value = queue.front();
		queue.pop();
		return value;
	}

	MqClient* makeClient() {
		return new MqClient(serverAddress, sslEnabled, sslCertFile); 
	}  

	ServerAddress getServerAddress() {
		return ServerAddress(serverAddress, sslEnabled);
	}

private:
	std::string serverAddress;
	bool sslEnabled;
	std::string sslCertFile;


	int size;
	int maxSize;
	std::queue<MqClient*> queue;
	mutable std::mutex mutex;
	std::condition_variable signal;
}; 

#endif