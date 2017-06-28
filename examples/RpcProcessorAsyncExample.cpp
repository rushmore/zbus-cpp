#include "RpcProcessor.h"
#include "Consumer.h"
 

class MyService2 {
public:
	int plus(int a, int b) {
		return a + b;
	}
	std::string getString(std::string str) {
		return str;
	}
};
  
class AsyncRpcProcessor : public RpcProcessor {
private:
	MyService2* svc; 
public:
	AsyncRpcProcessor(MyService2* svc) {
		this->svc = svc; 
	} 

	virtual void process(Request* req, Response* res) {
		if (req->method == "plus") {
			int a = stoi(req->params[0].asString());
			int b = stoi(req->params[1].asString());
			int c = svc->plus(a, b);
			res->result = Json::Value(c);
			return;
		}

		if (req->method == "getString") {
			std::string s = req->params[0].asString();
			s = svc->getString(s);
			res->result = Json::Value(s);
			return;
		}
	}

	virtual void handle(Message* reqMsg, MqClient* client) {
		Message* resMsg = new Message();
		resMsg->setId(reqMsg->getId());
		resMsg->setTopic(reqMsg->getTopic());
		resMsg->setRecver(reqMsg->getSender());

		Request* req = new Request();
		Response* res = new Response(); 
		req->fromJson(reqMsg->getBodyString());
		delete reqMsg; 

		//use thread to handle
		std::thread* t = new std::thread([=]() {
			try {
				process(req, res);
			}
			catch (std::exception& e) {
				res->error = Json::Value(e.what());
			}
			resMsg->setJsonBody(res->toJson());

			client->route(*resMsg);
			delete resMsg;
			delete req;
			delete res;  
		});   
		//TODO: delete thread once finshed, C++11 thread design sucks?
	}

};


int main(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  
	

	MyService2 svc;
	AsyncRpcProcessor rpcProcessor(&svc);

	Broker broker("localhost:15555");
	Consumer c(&broker, "MyRpc");  
	c.contextObject = &rpcProcessor;
	c.messageHander = RpcMessageHandler;
	c.start();  

	broker.join();
	return 0;
}