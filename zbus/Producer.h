#ifndef __ZBUS_PRODUCER_H__
#define __ZBUS_PRODUCER_H__  
 
#include "MqAdmin.h" 

namespace zbus {

	class ZBUS_API Producer : public MqAdmin {
	protected:
		ServerSelector produceSelector;
	public:
		Producer(Broker* broker) : MqAdmin(broker) {
			produceSelector = [](BrokerRouteTable& routeTable, Message& msg) {
				std::vector<ServerAddress> res;
				std::string topic = msg.getTopic();
				if (topic == "") {
					throw MqException("Message missing topic");
				}
				auto topicTable = routeTable.getTopicTable();
				if (topicTable.count(topic) < 1) {
					return res;
				}
				std::vector<TopicInfo>& topicServerList = topicTable[topic];
				if (topicServerList.size() < 1) return res;
				TopicInfo& target = topicServerList[0];
				for (TopicInfo& current : topicServerList) {
					if (current.consumerCount > target.consumerCount) {
						target = current;
					}
				}
				res.push_back(target.serverAddress);
				return res;
			};
		}

		Message produce(Message& msg, int timeout = 3000, ServerSelector selector = NULL) {
			if (selector == NULL) {
				selector = this->produceSelector;
			}
			std::vector<MqClientPool*> pools = broker->select(selector, msg);
			if (pools.size() < 0) throw new MqException("Missing MqServer for topic: " + msg.getTopic());
			MqClientPool* pool = pools[0];

			MqClient* client = NULL;
			client = pool->borrowClient();
			Message res = client->produce(msg, timeout);
			pool->returnClient(client);
			return res;
		}

		/**
		Need event loop facility to make the async work smoothingly, such as libuv from NodeJS
		*/
		void produceAsync(Message& msg, int timeout = 3000, ServerSelector selector = NULL) {
			msg.setAck(false);
			msg.setCmd(PROTOCOL_PRODUCE);

			if (selector == NULL) {
				selector = this->produceSelector;
			}
			std::vector<MqClientPool*> pools = broker->select(selector, msg);
			if (pools.size() < 0) throw new MqException("Missing MqServer for topic: " + msg.getTopic());
			MqClientPool* pool = pools[0];

			MqClient* client = NULL;
			client = pool->borrowClient();
			client->send(msg, timeout);
			pool->returnClient(client);
		}
	};

}//namespace
  
#endif