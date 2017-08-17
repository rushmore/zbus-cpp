#ifndef __ZBUS_KIT_H__
#define __ZBUS_KIT_H__   
   
#include "Message.h"
#include "Protocol.h"   
#include <json/json.h>

namespace zbus {  
	class ZBUS_API JsonKit {
	public: 
		static void parseConsumeGroupInfo(ConsumeGroupInfo& info, Message& msg) {
			Json::Value root;
			if (!parseBase(info, root, msg)) return;

			parseConsumeGroupInfo(info, root);
		}

		static void parseTopicInfo(TopicInfo& info, Message& msg) {
			Json::Value root;
			if (!parseBase(info, root, msg)) return;

			parseTopicInfo(info, root);
		}

		static void parseServerInfo(ServerInfo& info, Message& msg) {
			Json::Value root;
			if (!parseBase(info, root, msg)) return;

			parseServerInfo(info, root);
		}

		static void parseTrackerInfo(TrackerInfo& info, Message& msg) {
			Json::Value root;
			if (!parseBase(info, root, msg)) return;

			info.infoVersion = root["infoVersion"].asLargestInt();
			parseServerAddress(info.serverAddress, root["serverAddress"]);

			Json::Value& serverTableValue = root["serverTable"];
			std::vector<std::string>& serverAddressList = serverTableValue.getMemberNames();
			for (int i = 0; i < serverAddressList.size(); i++) {
				std::string& serverAddress = serverAddressList[i];
				Json::Value& serverInfoValue = serverTableValue[serverAddress];
				ServerInfo serverInfo;
				parseServerInfo(serverInfo, serverInfoValue);
				info.serverTable[serverAddress] = serverInfo;
			}

			info.serverVersion = root["serverVersion"].asString();
		}  


	private:
		static void parseServerAddress(ServerAddress& info, Json::Value& root) {
			info.address = root["address"].asString();
			info.sslEnabled = root["sslEnabled"].asBool();
		}

		static bool parseBase(ErrorInfo& info, Json::Value& root, Message& msg) {
			std::string bodyString = msg.getBodyString();
			if (msg.status != "200") {
				info.isError = true;
				info.error = MqException(bodyString);
				return false;
			}
			Json::Reader reader;
			reader.parse(bodyString, root);
			return true;
		}

		static void parseConsumeGroupInfo(ConsumeGroupInfo& info, Json::Value& root) {
			info.consumerCount = root["consumerCount"].asInt();
			for (Json::Value& value : root["consumerList"]) {
				info.consumerList.push_back(value.asString());
			}
			info.createdTime = root["createdTime"].asLargestInt();
			info.creator = root["creator"].asString();
			info.filter = root["filter"].asString();
			info.groupName = root["groupName"].asString();
			info.lastUpdatedTime = root["lastUpdatedTime"].asLargestInt();
			info.mask = root["mask"].asInt();
			info.messageCount = root["messageCount"].asLargestInt();
			info.topicName = root["topicName"].asString();
		}

		static void parseTopicInfo(TopicInfo& info, Json::Value& root) {
			for (Json::Value& value : root["consumeGroupList"]) {
				ConsumeGroupInfo groupInfo;
				parseConsumeGroupInfo(groupInfo, value);
				info.consumeGroupList.push_back(groupInfo);
			}
			info.consumerCount = root["consumerCount"].asInt();
			info.createdTime = root["createdTime"].asLargestInt();
			info.creator = root["creator"].asString();
			info.lastUpdatedTime = root["lastUpdatedTime"].asLargestInt();
			info.mask = root["mask"].asInt();
			info.messageDepth = root["messageDepth"].asLargestInt();
			parseServerAddress(info.serverAddress, root["serverAddress"]);
			info.serverVersion = root["serverVersion"].asString();
			info.topicName = root["topicName"].asString();
		}

		static void parseServerInfo(ServerInfo& info, Json::Value& root) {
			info.infoVersion = root["infoVersion"].asLargestInt();
			parseServerAddress(info.serverAddress, root["serverAddress"]);
			info.serverVersion = root["serverVersion"].asString();

			Json::Value& topicTableValue = root["topicTable"];
			std::vector<std::string>& topicNames = topicTableValue.getMemberNames();
			for (int i = 0; i < topicNames.size(); i++) {
				std::string& topicName = topicNames[i];
				Json::Value& topicInfoValue = topicTableValue[topicName];
				TopicInfo topicInfo;
				parseTopicInfo(topicInfo, topicInfoValue);
				info.topicTable[topicName] = topicInfo;
			}
		}
	};
}


#endif