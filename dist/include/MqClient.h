#ifndef __ZBUS_MQ_CLIENT_H__
#define __ZBUS_MQ_CLIENT_H__  
 
#include "MessageClient.h"
#include "Kit.h"  

#include <queue>
#include <mutex>
#include <condition_variable>

namespace zbus {

	class ZBUS_API ConsumeGroup {
	public:
		std::string groupName;
		std::string filter;          //filter on message'tag
		int mask = -1;

		std::string startCopy;       //create group from another group 
		int64_t startOffset = -1;
		std::string startMsgId;      //create group start from offset, msgId to check valid
		int64_t startTime = -1;    //create group start from time 

		void load(Message& msg) {
			groupName = msg.getConsumeGroup();
			filter = msg.getGroupFilter();
			mask = msg.getGroupMask();
			startCopy = msg.getGroupStartCopy();
			startOffset = msg.getGroupStartOffset();
			startMsgId = msg.getGroupStartMsgId();
			startTime = msg.getGroupStartTime();
		}

		void writeTo(Message& msg) {
			msg.setConsumeGroup(groupName);
			msg.setGroupFilter(filter);
			msg.setGroupMask(mask);
			msg.setGroupStartCopy(startCopy);
			msg.setGroupStartOffset(startOffset);
			msg.setGroupStartMsgId(startMsgId);
			msg.setGroupStartTime(startTime);
		}
	};

	class ZBUS_API MqClient : public MessageClient {

	public:
		std::string token;
		
		MqClient(std::string address, bool sslEnabled = false, std::string sslCertFile = "");
		virtual ~MqClient(); 

		Message produce(Message& msg, int timeout = 3000); 
		Message* consume(std::string topic, std::string group = "", int window = -1, int timeout = 3000);

		TrackerInfo queryTracker(int timeout = 3000);
		ServerInfo queryServer(int timeout = 3000); 
		TopicInfo queryTopic(std::string topic, int timeout = 3000); 
		ConsumeGroupInfo queryGroup(std::string topic, std::string group, int timeout = 3000); 
		TopicInfo declareTopic(std::string topic, int topicMask = -1, int timeout = 3000); 
		ConsumeGroupInfo declareGroup(std::string topic, std::string group, int timeout = 3000); 
		ConsumeGroupInfo declareGroup(std::string topic, ConsumeGroup& group, int timeout = 3000); 
		void removeTopic(std::string topic, int timeout = 3000); 
		void removeGroup(std::string topic, std::string group, int timeout = 3000);
		void emptyTopic(std::string topic, int timeout = 3000); 
		void emptyGroup(std::string topic, std::string group, int timeout = 3000); 
		void route(Message& msg, int timeout = 3000);
	};

	class ZBUS_API MqClientPool {
	public:
		MqClientPool(std::string serverAddress, int maxSize = 32, bool sslEnabled = false, std::string sslCertFile = "");
		virtual ~MqClientPool();

		void returnClient(MqClient* value); 
		MqClient* borrowClient(); 
		MqClient* makeClient(); 
		ServerAddress getServerAddress();

	private:
		std::string serverAddress;
		bool sslEnabled;
		std::string sslCertFile;


		int size;
		int maxSize;
		std::queue<MqClient*> queue;
		mutable std::mutex mutex;
		std::condition_variable signal;
	};

}//namespace

#endif