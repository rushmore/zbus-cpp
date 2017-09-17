#include "MqClient.h"

namespace zbus { 
	MqClient::MqClient(std::string address, bool sslEnabled, std::string sslCertFile) :
		MessageClient(address, sslEnabled, sslCertFile) { 
	}
	MqClient::~MqClient() { }


	Message MqClient::produce(Message& msg, int timeout) {
		msg.setCmd(PROTOCOL_PRODUCE);
		if (msg.getToken() == "") {
			msg.setToken(token);
		}
		Message* res = invoke(msg, timeout);
		Message ret(*res);
		delete res;
		return ret;
	}

	Message* MqClient::consume(std::string topic, std::string group, int window, int timeout) {
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

	TrackerInfo MqClient::queryTracker(int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_TRACKER);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TrackerInfo info;
		JsonKit::parseTrackerInfo(info, *res);

		if (res) delete res;
		return info;
	}

	ServerInfo MqClient::queryServer(int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_SERVER);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		ServerInfo info;
		JsonKit::parseServerInfo(info, *res);

		if (res) delete res;
		return info;
	}

	TopicInfo MqClient::queryTopic(std::string topic, int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_QUERY);
		msg.setTopic(topic);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TopicInfo info;
		JsonKit::parseTopicInfo(info, *res);

		if (res) delete res;
		return info;
	}

	ConsumeGroupInfo MqClient::queryGroup(std::string topic, std::string group, int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_QUERY);
		msg.setTopic(topic);
		msg.setConsumeGroup(group);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		ConsumeGroupInfo info;
		JsonKit::parseConsumeGroupInfo(info, *res);

		if (res) delete res;
		return info;
	}

	TopicInfo MqClient::declareTopic(std::string topic, int topicMask, int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_DECLARE);
		msg.setTopic(topic);
		msg.setTopicMask(topicMask);
		msg.setToken(token);
		Message* res = invoke(msg, timeout);

		TopicInfo info;
		JsonKit::parseTopicInfo(info, *res);

		if (res)  delete res;
		return info;
	}

	ConsumeGroupInfo MqClient::declareGroup(std::string topic, std::string group, int timeout) {
		ConsumeGroup consumeGroup;
		consumeGroup.groupName = group;
		return declareGroup(topic, consumeGroup, timeout);
	}

	ConsumeGroupInfo MqClient::declareGroup(std::string topic, ConsumeGroup& group, int timeout) {
		Message msg;
		msg.setCmd(PROTOCOL_DECLARE);
		msg.setTopic(topic);
		msg.setToken(token);
		group.writeTo(msg);

		Message* res = invoke(msg, timeout);

		ConsumeGroupInfo info;
		JsonKit::parseConsumeGroupInfo(info, *res);

		if (res)  delete res;

		return info;
	}

	void MqClient::removeTopic(std::string topic, int timeout) {
		removeGroup(topic, "", timeout);
	}

	void MqClient::removeGroup(std::string topic, std::string group, int timeout) {
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

	void MqClient::emptyTopic(std::string topic, int timeout) {
		emptyGroup(topic, "", timeout);
	}

	void MqClient::emptyGroup(std::string topic, std::string group, int timeout) {
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

	void MqClient::route(Message& msg, int timeout) {
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
	 

	MqClientPool::MqClientPool(std::string serverAddress, int maxSize, bool sslEnabled, std::string sslCertFile) :
		maxSize(maxSize),
		serverAddress(serverAddress), sslEnabled(sslEnabled), sslCertFile(sslCertFile)
	{

	}

	MqClientPool::~MqClientPool() {
		std::unique_lock<std::mutex> lock(mutex);
		while (!queue.empty()) {
			MqClient* client = queue.front();
			delete client;
			queue.pop();
		}
	}

	void MqClientPool::returnClient(MqClient* value) {
		if (value == NULL) return;
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.push(value);
		}
		signal.notify_one();
	}

	MqClient* MqClientPool::borrowClient() {
		std::unique_lock<std::mutex> lock(mutex);
		if (size < maxSize && queue.empty()) {
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

	MqClient* MqClientPool::makeClient() {
		return new MqClient(serverAddress, sslEnabled, sslCertFile);
	}

	ServerAddress MqClientPool::getServerAddress() {
		return ServerAddress(serverAddress, sslEnabled);
	} 
}//namespace