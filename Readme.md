                /\\\                                                                                               
                \/\\\                                                                                              
                 \/\\\                                                                /\\\          /\\\           
     /\\\\\\\\\\\ \/\\\         /\\\    /\\\  /\\\\\\\\\\               /\\\\\\\\     \/\\\         \/\\\          
     \///////\\\/  \/\\\\\\\\\  \/\\\   \/\\\ \/\\\//////              /\\\//////   /\\\\\\\\\\\  /\\\\\\\\\\\     
           /\\\/    \/\\\////\\\ \/\\\   \/\\\ \/\\\\\\\\\\            /\\\         \/////\\\///  \/////\\\///     
          /\\\/      \/\\\  \/\\\ \/\\\   \/\\\ \////////\\\           \//\\\            \/\\\         \/\\\       
         /\\\\\\\\\\\ \/\\\\\\\\\  \//\\\\\\\\\   /\\\\\\\\\\            \///\\\\\\\\     \///          \///       
         \///////////  \/////////    \/////////   \//////////               \////////                              


zbus strives to make Message Queue and Remote Procedure Call fast, light-weighted and easy to build your own service-oriented architecture for many different platforms. Simply put, zbus = mq + rpc.

zbus carefully designed on its protocol and components to embrace KISS(Keep It Simple and Stupid) principle, but in all it delivers power and elasticity. 


Start zbus, please refer to [https://gitee.com/rushmore/zbus](https://gitee.com/rushmore/zbus) 

# zbus-cpp
zbus's modern C++ client provides friendly and very easy API for C++.

## Getting started

## API Demo

Only demos the gist of API, more configurable usage calls for your further interest.

### Produce message

    Broker broker("localhost:15555");
	Producer p(&broker); 

	string topic = "MyTopic";
	p.declareTopic(topic);  

	Message msg;
	msg.setTopic(topic);
	msg.setBody("From C++ 11");

	p.produce(msg);



### Consume message

	Broker broker("localhost:15555");

	Consumer c(&broker, "MyTopic"); 
	c.messageHander = [](Message* msg, MqClient* client) {
		msg->print();
		delete msg;
	};  
	c.start();  

### RPC client

    Broker broker("localhost:15555"); 

	RpcInvoker rpc(&broker, "MyRpc");

	Request req;
	req.method = "plus";
	req.params.push_back(1);
	req.params.push_back(2); 

	Response res = rpc.invoke(req); 


### RPC service

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
		Logger::configDefaultLogger(0, LOG_DEBUG); 
		MyService svc;
		
		RpcProcessor p;            //You may configure thread pool size
		//p.modulePrefix;          //You may also configure on the default method prefix(module)
		registerMethods(p, &svc);  

		Broker broker("localhost:15555;localhost:15556");
		Consumer c(&broker, "MyRpc");  

		c.messageHander = [&p](Message* msg, MqClient* client) {
			p.handleAsync(msg, client);
		};

		c.connectionCount = 4;
		c.start();  

		broker.join();
		return 0;
	}