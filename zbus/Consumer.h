#ifndef __ZBUS_CONSUMER_H__
#define __ZBUS_CONSUMER_H__  
 
#include "MqAdmin.h" 

typedef void(*MessageHandler)(Message* , MqClient*, void* ctx);
class ConsumeThread {
public:
	ConsumeThread(MqClientPool* pool, MessageHandler messageHander=NULL, int connectionCount = 1, int timeout = 1000) :
		pool(pool), messageHander(messageHander), connectionCount(connectionCount), consumeTimeout(timeout){ 
	}

	~ConsumeThread() {
		this->close();
	}

	void start() {
		if (clientThreads.size() > 0) return;

		if (this->messageHander == NULL) {
			throw std::exception("Missing message handler");
		} 
		for (int i = 0; i < this->connectionCount; i++) {
			//separate recv and send clients since recv will block, otherwise consumer's message handler can not work in multi-threading environment
			MqClient* clientRecv = pool->makeClient();
			MqClient* clientSend = pool->makeClient();
			clients.push_back(clientRecv);
			clients.push_back(clientSend);
			std::thread* t = new std::thread(&ConsumeThread::run, this, clientRecv, clientSend);
			clientThreads.push_back(t);
		} 
	}

	void join() {
		for (std::thread* t : clientThreads) {
			t->join();
		}
	}

	void close() {
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

private:
	Message* take(MqClient* client) {
		Message* res = client->consume(topic, consumeGroup.groupName, consumeWindow, consumeTimeout);
		if (res->status == "404") {
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

	void run(MqClient* clientRecv, MqClient* clientSend) {
		while (running) {
			try {
				Message* msg = take(clientRecv);
				messageHander(msg, clientSend, contextObject);
			}
			catch (MqException& e) {   
				if (e.code == ERR_NET_RECV_FAILED) { //timeout?
					continue;
				} 
				clientRecv->close();
				clientSend->close();
				logger->error("%d, %s", e.code, e.message.c_str()); 
				std::this_thread::sleep_for(std::chrono::seconds(3));
			} 
		}
	}
public:
	std::string topic;
	ConsumeGroup consumeGroup;
	std::string token;
	int consumeWindow = 1;
	int consumeTimeout = 10000;
	int connectionCount = 1;
	MessageHandler messageHander;
	void* contextObject;

private:
	MqClientPool* pool;
	bool running = true;
	
	 
	std::vector<thread*> clientThreads;
	std::vector<MqClient*> clients;

	Logger* logger = Logger::getLogger(); 
};
 

class ZBUS_API Consumer : public MqAdmin {

public:
	Consumer(Broker* broker, string topic) : MqAdmin(broker) {
		consumeSelector = [](BrokerRouteTable& routeTable, Message& msg) {
			std::vector<ServerAddress> res;
			for (auto& kv : routeTable.serverTable) {
				res.push_back(kv.second.serverAddress);
			}
			return res;
		};
		this->topic = topic;
	}

	~Consumer() {
		this->close();
	}  

	void close() {
		for (auto& kv : consumeThreadTable) {
			ConsumeThread* ct = kv.second;
			delete ct;
		}
	}

	void start() {
		broker->contextObject = this;
		broker->onServerJoin = [](MqClientPool* pool, void* ctx) {
			Consumer* c = (Consumer*)ctx;
			c->startConsumeThread(pool);
		};
		broker->onServerLeave = [](ServerAddress serverAddress, void* ctx) {
			Consumer* c = (Consumer*)ctx;
			c->stopConsumeThread(serverAddress);
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

	void join() {
		for (auto& kv : consumeThreadTable) {
			kv.second->join();
		}
	}
	
private:
	void startConsumeThread(MqClientPool* pool) {
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
		ct->contextObject = this->contextObject;

		consumeThreadTable[serverAddress] = ct;
		ct->start();
	}

	void stopConsumeThread(ServerAddress serverAddress) {
		if (consumeThreadTable.count(serverAddress) <= 0) {
			return;
		}
		ConsumeThread* ct = consumeThreadTable[serverAddress];
		delete ct;
		consumeThreadTable.erase(serverAddress);
	}

private:
	std::map<ServerAddress, ConsumeThread*> consumeThreadTable;
protected:
	ServerSelector consumeSelector;

public:  
	std::string topic;
	ConsumeGroup consumeGroup;
	std::string token;
	int consumeWindow = 1;
	int consumeTimeout = 10000;
	int connectionCount = 1;
	MessageHandler messageHander;
	void* contextObject;
};
  
#endif