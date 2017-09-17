#ifndef __ZBUS_BROKER_H__
#define __ZBUS_BROKER_H__  
 
#include "MqClient.h" 
#include "Kit.h"
#include "Logger.h"
#include <map>
#include <string>   
#include <vector> 
#include <algorithm> 

namespace zbus {
	class Vote {
	public:
		int64_t version = 0;
		std::vector<ServerAddress> servers;
	};

	class ZBUS_API BrokerRouteTable { 
	public:
		BrokerRouteTable(double voteFactor = 0.5);
		std::map<ServerAddress, ServerInfo> getServerTable();
		std::map<std::string, std::vector<TopicInfo>> getTopicTable();
		std::vector<ServerAddress> updateTracker(TrackerInfo& trackerInfo);
		std::vector<ServerAddress> removeTracker(ServerAddress trackerAddress);
	private:
		
		std::vector<ServerAddress> purge();

	private:
		mutable std::mutex serverTableMutex;
		mutable std::mutex topicTableMutex;
		mutable std::mutex votesTableMutex;
		mutable std::mutex votedTrackerMutex;

		std::map<ServerAddress, ServerInfo> serverTable;
		std::map<std::string, std::vector<TopicInfo>> topicTable;
		std::map<ServerAddress, Vote> votesTable;
		std::map<ServerAddress, bool> votedTrackers;

		double voteFactor = 0.5;
	}; 
	

	typedef std::function<std::vector<ServerAddress>(BrokerRouteTable&, Message&)> ServerSelector; 

	class ZBUS_API Broker {
	public:
		std::function<void(MqClientPool*)> onServerJoin;
		std::function<void(ServerAddress serverAddress)> onServerLeave;
		BrokerRouteTable routeTable;

	public:
		Broker(std::string trackerAddress, int waitReady = 3, int poolSize = 32);
		virtual ~Broker();

		std::vector<MqClientPool*> select(ServerSelector selector, Message& msg);

		void addTracker(ServerAddress& serverAddress, std::string sslCertFile = "");
		void addServer(ServerAddress serverAddress);
		void removeServer(ServerAddress serverAddress);
		void join();
		
	private:
		Logger* logger = Logger::getLogger();

		std::map<ServerAddress, MqClientPool*> poolTable;
		std::map<ServerAddress, MqClient*> trackerSubscribers;
		std::map<std::string, std::string> sslCertFileTable;

		std::condition_variable readySignal;
		mutable std::mutex readyMutex;

		int poolSize = 32;
	};  

}//namespace

#endif