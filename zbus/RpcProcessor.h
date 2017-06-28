#ifndef __ZBUS_RPC_PROCESSOR_H__
#define __ZBUS_RPC_PROCESSOR_H__  
 
#include "RpcInvoker.h" 

class ZBUS_API RpcProcessor { 
public: 
	virtual void process(Request* req, Response* res) = 0;
	
	virtual void handle(Message* reqMsg, MqClient* client) {
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
};  

ZBUS_API inline void RpcMessageHandler(Message* msg, MqClient* client, void* ctx) {
	RpcProcessor* p = (RpcProcessor*)ctx;
	p->handle(msg, client);
};


#endif
