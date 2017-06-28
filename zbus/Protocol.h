#ifndef __ZBUS_PROTOCOL_H__
#define __ZBUS_PROTOCOL_H__  
 
#include <string>
#include <map>
#include <vector>
#include <cstdint>
using namespace std;


#define PROTOCOL_VERSION_VALUE "0.8.0"  //start from 0.8.0 

//=============================[1] Command Values================================================
//MQ Produce/Consume
#define PROTOCOL_PRODUCE "produce"
#define PROTOCOL_CONSUME "consume"
#define PROTOCOL_ROUTE  "route"     //route back message to sender, designed for RPC 
#define PROTOCOL_RPC  "rpc"       //the same as produce command except rpc set ack false by default

//Topic control
#define PROTOCOL_DECLARE  "declare"
#define PROTOCOL_QUERY  "query"
#define PROTOCOL_REMOVE  "remove"
#define PROTOCOL_EMPTY  "empty"

//High Availability (HA) 
#define PROTOCOL_TRACK_PUB  "track_pub"
#define PROTOCOL_TRACK_SUB  "track_sub"
#define PROTOCOL_TRACKER  "tracker"
#define PROTOCOL_SERVER  "server"


//=============================[2] Parameter Values================================================
#define PROTOCOL_CMD  "cmd"
#define PROTOCOL_TOPIC  "topic"
#define PROTOCOL_TOPIC_MASK  "topic_mask"
#define PROTOCOL_TAG  "tag"
#define PROTOCOL_OFFSET  "offset"

#define PROTOCOL_CONSUME_GROUP  "consume_group"
#define PROTOCOL_GROUP_START_COPY  "group_start_copy"
#define PROTOCOL_GROUP_START_OFFSET  "group_start_offset"
#define PROTOCOL_GROUP_START_MSGID  "group_start_msgid"
#define PROTOCOL_GROUP_START_TIME  "group_start_time"
#define PROTOCOL_GROUP_FILTER  "group_filter"
#define PROTOCOL_GROUP_MASK  "group_mask"
#define PROTOCOL_CONSUME_WINDOW  "consume_window"

#define PROTOCOL_SENDER  "sender"
#define PROTOCOL_RECVER  "recver"
#define PROTOCOL_ID  "id"

#define PROTOCOL_HOST  "host"
#define PROTOCOL_ACK  "ack"
#define PROTOCOL_ENCODING  "encoding"

#define PROTOCOL_ORIGIN_ID  "origin_id"
#define PROTOCOL_ORIGIN_URL  "origin_url"
#define PROTOCOL_ORIGIN_STATUS  "origin_status"

//Security 
#define PROTOCOL_TOKEN "token"

#define PROTOCOL_MASK_PAUSE           (1 << 0)
#define PROTOCOL_MASK_RPC             (1 << 1)
#define PROTOCOL_MASK_EXCLUSIVE       (1 << 2)
#define PROTOCOL_MASK_DELETE_ON_EXIT  (1 << 3)



class ZBUS_API ServerAddress {
public:
	string address;
	bool sslEnabled;
	ServerAddress() {

	}
	ServerAddress(char* address, bool sslEnabled = false) {
		this->address = address;
		this->sslEnabled = sslEnabled;
	}

	ServerAddress(string& address, bool sslEnabled = false) {
		this->address = address;
		this->sslEnabled = sslEnabled;
	}

	ServerAddress(ServerAddress* serverAddress) {
		this->address = serverAddress->address;
		this->sslEnabled = serverAddress->sslEnabled;
	}  

	virtual bool operator==(const ServerAddress& other) const {
		return this->address == other.address && this->sslEnabled == other.sslEnabled;
	}

	std::string toString() {
		if (this->sslEnabled) return "SSL" + this->address;
		return this->address;
	}
};

inline static bool operator<(const ServerAddress& l, const ServerAddress& r) {
	if (l.address < r.address) return true;
	if (l.address > r.address) return false;
	return l.sslEnabled < r.sslEnabled;
}


class ZBUS_API ErrorInfo {  //used only for batch operation indication
public: 
	bool isError;
	std::exception error;

	void setError(std::exception& error) {
		isError = true;
		this->error = error;
	}
};

class ZBUS_API TrackItem : public ErrorInfo {
public:
	TrackItem() {}
	ServerAddress serverAddress;
	string serverVersion;
};

class ZBUS_API ConsumeGroupInfo : public ErrorInfo {
public:
	string topicName;
	string groupName;
	int mask;
	string filter;
	int64_t messageCount;
	int consumerCount;
	vector<string> consumerList;

	string creator;
	int64_t createdTime;
	int64_t lastUpdatedTime;
};

class ZBUS_API TopicInfo : public TrackItem {
public:
	string topicName;
	int mask; 
	int64_t messageDepth; 
	int consumerCount; 
	vector<ConsumeGroupInfo> consumeGroupList; 

	string creator;
	int64_t createdTime;
	int64_t lastUpdatedTime;
};

class ZBUS_API ServerInfo : public TrackItem {
public:
	int64_t infoVersion;
	map<string, TopicInfo> topicTable; 
};

class ZBUS_API TrackerInfo : public TrackItem {
public:
	int64_t infoVersion;
	map<string, ServerInfo> serverTable; 
};  


class ZBUS_API MqException : public exception {
public:
	int code;
	string message;

	MqException(string message = "Unknown exception", int code = -1) :
		exception(message.c_str())
	{
		this->message = message;
		this->code = code;
	}

	MqException(MqException& ex) :
		exception(ex.message.c_str())
	{
		this->code = ex.code;
		this->message = ex.message;
	}
};

#endif