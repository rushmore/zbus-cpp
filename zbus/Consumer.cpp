#include "Consumer.h"


namespace zbus {
	 
	ConsumeThread::ConsumeThread(MqClientPool* pool, std::function<void(Message*, MqClient*)> messageHander, 
		int connectionCount, int timeout) :
		pool(pool), messageHander(messageHander), 
		connectionCount(connectionCount), 
		consumeTimeout(timeout) {
	}

	ConsumeThread::~ConsumeThread() {
		this->close();
	}

	void ConsumeThread::start() {
		if (clientThreads.size() > 0) return;

		if (this->messageHander == NULL) {
			throw std::exception("Missing message handler");
		}
		for (int i = 0; i < this->connectionCount; i++) {
			MqClient* client = pool->makeClient();
			clients.push_back(client);
			std::thread* t = new std::thread(&ConsumeThread::run, this, client);
			clientThreads.push_back(t);
		}
	}

	void ConsumeThread::join() {
		for (std::thread* t : clientThreads) {
			t->join();
		}
	}

	void ConsumeThread::close() {
		running = false;
		for (MqClient* client : clients) {
			client->close();
		}
		for (std::thread* t : clientThreads) {
			t->join();
			delete t;
		}
		clientThreads.clear();
		for (MqClient* client : clients) {
			delete client;
		}
		clients.clear();
	}
	 
	Message* ConsumeThread::take(MqClient* client) {
		Message* res = client->consume(topic, consumeGroup.groupName, consumeWindow, consumeTimeout);
		if (res->status == "404") {
			client->declareTopic(topic, topicMask);
			client->declareGroup(topic, consumeGroup);
			delete res;
			return take(client);
		}
		if (res->status == "200") {
			res->setId(res->getOriginId());
			res->removeHeader(PROTOCOL_ORIGIN_ID);
			if (res->getOriginUrl() != "") {
				res->url = res->getOriginUrl();
				res->removeHeader(PROTOCOL_ORIGIN_URL);
				res->status = "";
			}
		}
		return res;
	}

	void ConsumeThread::run(MqClient* client) {
		while (running) {
			try {
				Message* msg = take(client);
				messageHander(msg, client);
			}
			catch (MqException& e) {
				if (e.code == ERR_NET_RECV_FAILED) { //timeout?
					continue;
				}
				client->close();
				logger->error("%d, %s", e.code, e.message.c_str());
				break;
			}
		}
	} 
	 
	Consumer::Consumer(Broker* broker, std::string topic) : MqAdmin(broker) {
		consumeSelector = [](BrokerRouteTable& routeTable, Message& msg) {
			std::vector<ServerAddress> res;
			for (auto& kv : routeTable.getServerTable()) {
				res.push_back(kv.second.serverAddress);
			}
			return res;
		};
		this->topic = topic;
	}

	Consumer::~Consumer() {
		this->close();
	}

	void Consumer::close() {
		for (auto& kv : consumeThreadTable) {
			ConsumeThread* ct = kv.second;
			delete ct;
		}
	}

	void Consumer::start() {
		broker->onServerJoin = [this](MqClientPool* pool) {
			this->startConsumeThread(pool);
		};
		broker->onServerLeave = [this](ServerAddress serverAddress) {
			this->stopConsumeThread(serverAddress);
		};
		Message msgCtrl;
		msgCtrl.setTopic(this->topic);
		msgCtrl.setToken(this->token);
		consumeGroup.writeTo(msgCtrl);
		std::vector<MqClientPool*> pools = broker->select(this->consumeSelector, msgCtrl);
		for (MqClientPool* pool : pools) {
			this->startConsumeThread(pool);
		}
	}

	void Consumer::join() {
		for (auto& kv : consumeThreadTable) {
			kv.second->join();
		}
	} 

	void Consumer::startConsumeThread(MqClientPool* pool) {
		std::unique_lock<std::mutex> lock(threadMutex);
		ServerAddress serverAddress = pool->getServerAddress();
		if (consumeThreadTable.count(serverAddress) > 0) {
			return;
		}
		ConsumeThread* ct = new ConsumeThread(pool);
		ct->connectionCount = this->connectionCount;
		ct->consumeGroup = this->consumeGroup;
		ct->consumeTimeout = this->consumeTimeout;
		ct->consumeWindow = this->consumeWindow;
		ct->messageHander = this->messageHander;
		ct->token = this->token;
		ct->topic = this->topic;
		ct->topicMask = this->topicMask;

		consumeThreadTable[serverAddress] = ct;
		ct->start();
	}

	void Consumer::stopConsumeThread(ServerAddress serverAddress) {
		std::unique_lock<std::mutex> lock(threadMutex);

		if (consumeThreadTable.count(serverAddress) <= 0) {
			return;
		}
		ConsumeThread* ct = consumeThreadTable[serverAddress];
		delete ct;
		consumeThreadTable.erase(serverAddress);
	} 

}//namespace