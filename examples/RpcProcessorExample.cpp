#include "RpcProcessor.h"
#include "Consumer.h"
#include <vector>


using namespace zbus;
using namespace std;

//businesss object
class MyService {
public:
	int plus(int a, int b) {
		return a + b;
	}
	std::string getString(std::string str) {
		return str;
	}
};
 

//NO reflection in C++, you need to wrap in this way, ugly? 
void registerMethods(RpcProcessor& p, MyService* svc) {
	
	p.addMethod("plus", [svc](vector<Json::Value>& params) { 
		int a = stoi(params[0].asString());
		int b = stoi(params[1].asString());
		int c = svc->plus(a, b);
		return Json::Value(c);
	});


	p.addMethod("getString", [svc](vector<Json::Value>& params) { 
		std::string str = params[0].asString();
		std::string res = svc->getString(str);
		return Json::Value(res);
	});
}


int main(int argc, char* argv[]) {
	Logger::configDefaultLogger(NULL, LOG_DEBUG); 
	MyService svc;
	
	RpcProcessor p;            //You may configure thread pool size
	//p.modulePrefix;          //You may also configure on the default method prefix(module)
	registerMethods(p, &svc);  

	Broker broker("localhost:15555;localhost:15556");
	Consumer c(&broker, "MyRpc");  
	c.topicMask = PROTOCOL_MASK_MEMORY | PROTOCOL_MASK_RPC;

	c.messageHander = [&p](Message* msg, MqClient* client) {
		p.handleAsync(msg, client);
	};

	c.connectionCount = 4;
	c.start();  

	broker.join();
	return 0;
}