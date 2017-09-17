#ifndef __ZBUS_CONSUMER_H__
#define __ZBUS_CONSUMER_H__  
 
#include "MqAdmin.h" 

namespace zbus {
	 
	class ZBUS_API ConsumeThread {
	public:
		std::string topic;
		int topicMask = 0;
		ConsumeGroup consumeGroup;
		std::string token;
		int consumeWindow = 1;
		int consumeTimeout = 60000;
		int connectionCount = 1;
		std::function<void(Message*, MqClient*)> messageHander;

	public:
		ConsumeThread(MqClientPool* pool, std::function<void(Message*, MqClient*)> messageHander = NULL, 
			int connectionCount = 1, int timeout = 1000); 
		~ConsumeThread();

		void start(); 
		void join(); 
		void close();

	private:
		Message* take(MqClient* client); 
		void run(MqClient* client);
	
	private:
		MqClientPool* pool;
		bool running = true; 

		std::vector<std::thread*> clientThreads;
		std::vector<MqClient*> clients;

		Logger* logger = Logger::getLogger();
	};


	class ZBUS_API Consumer : public MqAdmin {
	public:
		std::string topic;
		int topicMask;
		ConsumeGroup consumeGroup;
		std::string token;
		int consumeWindow = 1;
		int consumeTimeout = 10000;
		int connectionCount = 1;
		std::function<void(Message*, MqClient*)> messageHander;

	public:
		Consumer(Broker* broker, std::string topic); 
		~Consumer();

		void close(); 
		void start(); 
		void join();

	private:
		void startConsumeThread(MqClientPool* pool);
		void stopConsumeThread(ServerAddress serverAddress);

	protected:
		ServerSelector consumeSelector;

	private:
		std::map<ServerAddress, ConsumeThread*> consumeThreadTable;
		mutable std::mutex threadMutex;
	};

}//namespace
  
#endif