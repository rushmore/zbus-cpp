#include "RpcProcessor.h"
#include "Consumer.h"
 

class MyService {
public:
	int plus(int a, int b) {
		return a + b;
	}
	std::string getString(std::string str) {
		return str;
	} 
};

class MyRpcProcessor : public RpcProcessor {
private:
	MyService* svc;

public:
	MyRpcProcessor(MyService* svc) {
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

		res->error = Json::Value("Missing method: " + req->method);
	}
};



int main_RpcProcessor(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);  

	MyService svc;
	MyRpcProcessor rpcProcessor(&svc);


	Broker broker("localhost:15555");
	Consumer c(&broker, "MyRpc"); 
	c.contextObject = &rpcProcessor; //RpcMessageHandler need this context, C++ enclosure issue
	c.messageHander = RpcMessageHandler;
	c.start();  

	broker.join();
	return 0;
}