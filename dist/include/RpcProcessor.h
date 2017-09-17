#ifndef __ZBUS_RPC_PROCESSOR_H__
#define __ZBUS_RPC_PROCESSOR_H__  
 
#include "RpcInvoker.h"
#include "ThreadPool.h"

#include <map>
#include <mutex> 

namespace zbus {

	typedef std::function<Json::Value(std::vector<Json::Value>&)> Method;

	class ZBUS_API RpcProcessor { 
	public:
		std::vector<std::string> modulePrefix;
		ThreadPool threadPool;
		RpcProcessor(int threadPoolSize = 1);
		void addMethod(std::string methodName, Method method, std::string module = "");

		void handleAsync(Message* reqMsg, MqClient* client); 
		virtual void handle(Message* reqMsg, MqClient* client);
		virtual void process(Request* req, Response* res);

	protected:
		virtual Method findMethod(Request* req);
	private:
		std::map<std::string, Method> methodTable;
		mutable std::mutex methodMutex; 
	}; 

}
#endif
