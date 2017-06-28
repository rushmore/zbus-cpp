#ifndef __ZBUS_RPCINVOKER_H__
#define __ZBUS_RPCINVOKER_H__  
 
#include "Producer.h"
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

class ZBUS_API Request { 
public:
	std::string method;
	std::string module;
	std::vector<Json::Value> params; 

	std::string toJson() {
		Json::Value root;
		root["method"] = method;
		root["module"] = module;
		Json::Value args;
		for (Json::Value& param : this->params) {
			args.append(param);
		}
		root["params"] = args;

		Json::StyledWriter writer;
		return writer.write(root);
	}

	void fromJson(std::string& jsonString) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(jsonString, root); 
		this->method = root["method"].asString();
		this->module = root["module"].asString();
		for (Json::Value& param : root["params"]) {
			this->params.push_back(param);
		}
	}
};

class ZBUS_API Response {
public:
	Json::Value result;
	Json::Value error;

	bool isError() {
		return !error.isNull();
	}

	std::string toJson() {
		Json::Value root;
		root["result"] = result;
		root["error"] = error; 
		Json::StyledWriter writer;
		return writer.write(root);
	}

	void fromJson(std::string& jsonString) {
		Json::Value root;
		Json::Reader reader;
		reader.parse(jsonString, root);
		this->result = root["result"];
		this->error = root["error"];
	}
};

class ZBUS_API RpcInvoker {
public:
	RpcInvoker(Broker* broker, std::string topic, int timeout = 3000, ServerSelector selector = NULL) :
		producer(broker){
		this->broker = broker;
		this->topic = topic;
		this->timeout = timeout;
		this->rpcSelector = selector; 
	}
	
	Response invoke(Request& req, int timeout = 3000, ServerSelector selector = NULL) {
		Message msg;
		msg.setAck(false);
		msg.setTopic(topic);
		msg.setToken(token); 
		 
		msg.setJsonBody(req.toJson());
		
		if (selector == NULL) {
			selector = rpcSelector;
		}
		Message msgRes = producer.produce(msg, timeout, selector);
		Response res;
		std::string bodyString = msgRes.getBodyString();
		if (msgRes.status != "200") {
			res.error = Json::Value(bodyString);
		}
		else {
			res.fromJson(bodyString);
		}
		return res;
	}
	
private:
	Broker* broker;
	std::string topic;
	std::string token;
	int timeout;
	ServerSelector rpcSelector;
	Producer producer;
};



#endif
