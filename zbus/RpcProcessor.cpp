#include "RpcProcessor.h"


namespace zbus { 
	 
	RpcProcessor::RpcProcessor(int threadPoolSize) :threadPool(threadPoolSize) {

	} 
	void RpcProcessor::addMethod(std::string methodName, Method method, std::string module) {
		std::unique_lock<std::mutex> lock(methodMutex);
		if (module != "") {
			this->methodTable[module + "." + methodName] = method;
			return;
		}

		this->methodTable[methodName] = method;
		for (auto prefix : modulePrefix) {
			std::string module = prefix + "." + methodName;
			this->methodTable[module] = method;
		}
	}

	void RpcProcessor::handleAsync(Message* reqMsg, MqClient* client) {
		threadPool.submit([this, reqMsg, client]() {
			this->handle(reqMsg, client);
		});
	}

	void RpcProcessor::handle(Message* reqMsg, MqClient* client) {
		Message* resMsg = new Message();
		resMsg->setId(reqMsg->getId());
		resMsg->setTopic(reqMsg->getTopic());
		resMsg->setRecver(reqMsg->getSender());

		Request req;
		Response res;
		std::string body = reqMsg->getBodyString();
		req.fromJson(body);
		delete reqMsg;

		try {
			process(&req, &res);
		}
		catch (std::exception& e) {
			res.error = Json::Value(e.what());
		}
		resMsg->setJsonBody(res.toJson());

		client->route(*resMsg);
		delete resMsg;
	}

	void RpcProcessor::process(Request* req, Response* res) {
		Method m = findMethod(req);
		if (m == NULL) {
			res->error = Json::Value("Missing method: " + req->method);
			return;
		}
		try {
			res->result = m(req->params);
		}
		catch (std::exception& e) {
			res->error = Json::Value(e.what());
		}
	}

	Method RpcProcessor::findMethod(Request* req) {
		std::unique_lock<std::mutex> lock(methodMutex);
		std::string fullName = req->method;
		if (req->module != "") {
			fullName += "." + req->method;
		}
		auto iter = methodTable.find(fullName);
		if (iter != methodTable.end()) {
			return iter->second;
		}
		return NULL;
	}  
}