#ifndef __ZBUS_KIT_H__
#define __ZBUS_KIT_H__   
   
#include "Message.h"
#include "Protocol.h"   
#include <json/json.h>

namespace zbus {  
	class ZBUS_API JsonKit {
	public: 
		static void parseConsumeGroupInfo(ConsumeGroupInfo& info, Message& msg); 
		static void parseTopicInfo(TopicInfo& info, Message& msg); 
		static void parseServerInfo(ServerInfo& info, Message& msg); 
		static void parseTrackerInfo(TrackerInfo& info, Message& msg);


	private:
		static void parseServerAddress(ServerAddress& info, Json::Value& root); 
		static bool parseBase(ErrorInfo& info, Json::Value& root, Message& msg); 
		static void parseConsumeGroupInfo(ConsumeGroupInfo& info, Json::Value& root); 
		static void parseTopicInfo(TopicInfo& info, Json::Value& root); 
		static void parseServerInfo(ServerInfo& info, Json::Value& root);
	};
}


#endif