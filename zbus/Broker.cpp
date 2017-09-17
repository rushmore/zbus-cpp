#include "Broker.h"
namespace zbus {

	BrokerRouteTable::BrokerRouteTable(double voteFactor) : voteFactor(voteFactor) {

	}

	std::map<ServerAddress, ServerInfo> BrokerRouteTable::getServerTable() {
		std::unique_lock<std::mutex> lock(serverTableMutex);
		return serverTable;
	}

	std::map<std::string, std::vector<TopicInfo>> BrokerRouteTable::getTopicTable() {
		std::unique_lock<std::mutex> lock(topicTableMutex);
		return topicTable;
	} 
	
	std::vector<ServerAddress> BrokerRouteTable::updateTracker(TrackerInfo& trackerInfo) {
		{
			std::unique_lock<std::mutex> lock(votedTrackerMutex);
			votedTrackers[trackerInfo.serverAddress] = true;
		}
		//1) update votes
		{
			std::unique_lock<std::mutex> lock(votesTableMutex);
			ServerAddress& trackerAddress = trackerInfo.serverAddress;
			int64_t trackerVersion = trackerInfo.infoVersion;
			Vote vote;
			if (votesTable.count(trackerAddress)) {
				vote = votesTable[trackerAddress];
			}
			if (trackerVersion <= vote.version) {
				return std::vector<ServerAddress>();
			}

			vote.version = trackerVersion;
			std::vector<ServerAddress> servers;
			for (auto& kv : trackerInfo.serverTable) {
				servers.push_back(kv.second.serverAddress);
			}
			vote.servers = servers;
			votesTable[trackerAddress] = vote;
		}
		//2) merge server table 
		{
			std::unique_lock<std::mutex> lock(serverTableMutex);
			for (auto& kv : trackerInfo.serverTable) {
				ServerInfo& serverInfo = kv.second;
				if (serverTable.count(serverInfo.serverAddress) > 0) {
					ServerInfo& oldServerInfo = serverTable[serverInfo.serverAddress];
					if (oldServerInfo.infoVersion >= serverInfo.infoVersion) {
						continue;
					}
				}
				serverTable[serverInfo.serverAddress] = serverInfo;
			}
		}
		//3) purge
		return purge();
	}


	std::vector<ServerAddress> BrokerRouteTable::removeTracker(ServerAddress trackerAddress) {
		{
			std::unique_lock<std::mutex> lock(votesTableMutex);
			if (votesTable.count(trackerAddress) < 1) {
				return std::vector<ServerAddress>();
			}
			votesTable.erase(trackerAddress);
		}

		return purge();
	}
	 
	std::vector<ServerAddress> BrokerRouteTable::purge() {
		std::unique_lock<std::mutex> lock(serverTableMutex);

		std::vector<ServerAddress> toRemove;
		for (auto& s : serverTable) {
			ServerInfo& serverInfo = s.second;
			ServerAddress& serverAddress = serverInfo.serverAddress;
			int count = 0;
			for (auto& v : votesTable) {
				Vote& vote = v.second;
				std::vector<ServerAddress>& servers = vote.servers;
				if (std::find(servers.begin(), servers.end(), serverAddress) != servers.end()) {
					count++;
				}
			}
			int totalCount = 0;
			{
				std::unique_lock<std::mutex> lock(votedTrackerMutex);
				totalCount = votedTrackers.size();
			}
			if (count < totalCount*voteFactor) {
				toRemove.push_back(serverAddress);
			}
		}

		for (ServerAddress& addr : toRemove) {
			serverTable.erase(addr);
		}

		std::map<std::string, std::vector<TopicInfo>> topicTableLocal;
		for (auto& s : serverTable) {
			ServerInfo& serverInfo = s.second;
			for (auto& t : serverInfo.topicTable) {
				TopicInfo& topicInfo = t.second;
				std::string& topicName = topicInfo.topicName;
				if (topicTableLocal.count(topicName) < 1) {
					topicTableLocal[topicName] = std::vector<TopicInfo>();
				}
				topicTableLocal[topicName].push_back(topicInfo);
			}
		}
		{
			std::unique_lock<std::mutex> lock(topicTableMutex);
			this->topicTable = topicTableLocal;
		}
		return toRemove;
	} 

	///////////////////////////Broker////////////////////////////////////

	Broker::Broker(std::string trackerAddress, int waitReady, int poolSize) {
		this->poolSize = poolSize;
		char* splitToken;
		char* p = strtok_s((char*)trackerAddress.c_str(), ";", &splitToken);
		int count = 0;
		while (p) {
			std::string addr = p;
			if (addr == "") break;
			ServerAddress serverAddress(addr, false);
			addTracker(serverAddress);
			count++;
			p = strtok_s(NULL, ";", &splitToken);
		}
		if (count > 0) {
			std::unique_lock<std::mutex> lock(readyMutex);
			readySignal.wait_for(lock, std::chrono::seconds(waitReady));
		}
	}

	Broker::~Broker() {
		readySignal.notify_all();
		for (auto& kv : poolTable) {
			delete kv.second;
		}
		poolTable.clear();

		for (auto& kv : trackerSubscribers) {
			delete kv.second;
		}
		trackerSubscribers.clear();
	}

	void Broker::addTracker(ServerAddress& serverAddress, std::string sslCertFile) {
		if (trackerSubscribers.count(serverAddress)) {
			return;
		}
		if (sslCertFile != "") {
			sslCertFileTable[serverAddress.address] = sslCertFile;
		}
		MqClient* client = new MqClient(serverAddress.address, serverAddress.sslEnabled, sslCertFile);
		trackerSubscribers[serverAddress] = client;

		client->onConnected = [](MessageClient* client) {
			Message msg;
			msg.setCmd(PROTOCOL_TRACK_SUB);
			client->send(msg);
		};

		client->onDisconnected = [this, client]() {
			std::vector<ServerAddress> toRemove = this->routeTable.removeTracker(client->serverAddress);
			for (ServerAddress serverAddress : toRemove) {
				this->removeServer(serverAddress);
			}
		};

		client->onMessage = [this, client](Message* msg) {
			if (msg->status != "200") {
				delete msg;
				return;
			}

			TrackerInfo info;
			JsonKit::parseTrackerInfo(info, *msg);
			delete msg;

			client->serverAddress = info.serverAddress;
			BrokerRouteTable& routeTable = this->routeTable;
			std::vector<ServerAddress> toRemove = routeTable.updateTracker(info);
			std::map<ServerAddress, ServerInfo> serverTable = routeTable.getServerTable();

			for (auto& kv : serverTable) {
				ServerInfo& serverInfo = kv.second;
				this->addServer(serverInfo.serverAddress);
			}
			for (ServerAddress serverAddress : toRemove) {
				this->removeServer(serverAddress);
			}
			this->readySignal.notify_all();
		};

		client->start();
	}

	void Broker::join() {
		for (auto& kv : trackerSubscribers) {
			kv.second->join();
		}
	}

	void Broker::addServer(ServerAddress serverAddress) {
		if (poolTable.count(serverAddress) > 0) {
			return;
		}
		logger->info("%s joined", serverAddress.toString().c_str());
		std::string sslCertFile = sslCertFileTable[serverAddress.address];
		MqClientPool* pool = new MqClientPool(serverAddress.address, poolSize, serverAddress.sslEnabled, sslCertFile);
		poolTable[serverAddress] = pool;
		if (onServerJoin) {
			onServerJoin(pool);
		}
	}

	void Broker::removeServer(ServerAddress serverAddress) {
		if (poolTable.count(serverAddress) < 1) {
			return;
		}
		if (onServerLeave) {
			onServerLeave(serverAddress);
		}
		MqClientPool* pool = poolTable[serverAddress];
		poolTable.erase(serverAddress);
		if (pool != NULL) {
			delete pool;
		}

		logger->info("%s left", serverAddress.toString().c_str());
	}


	std::vector<MqClientPool*> Broker::select(ServerSelector selector, Message& msg) {
		std::vector<ServerAddress> addrList = selector(this->routeTable, msg);
		std::vector<MqClientPool*> res;
		for (ServerAddress& address : addrList) {
			auto kv = poolTable.find(address);
			if (kv == poolTable.end()) {
				continue;
			}
			res.push_back(kv->second);
		}
		return res;
	}  
}