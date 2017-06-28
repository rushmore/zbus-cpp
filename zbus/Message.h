#ifndef __ZBUS_MESSAGE_H__
#define __ZBUS_MESSAGE_H__   

#include "Platform.h"
#include "Protocol.h"
#include "Buffer.h" 
#include <sstream>
using namespace std;

class ZBUS_API Message {
public:
	string getCmd() {
		return getHeader(PROTOCOL_CMD);
	}
	void setCmd(string value) {
		setHeader(PROTOCOL_CMD, value);
	}  
	string getTopic() {
		return getHeader(PROTOCOL_TOPIC);
	}
	void setTopic(string value) {
		setHeader(PROTOCOL_TOPIC, value);
	}
	string getConsumeGroup() {
		return getHeader(PROTOCOL_CONSUME_GROUP);
	}
	void setConsumeGroup(string value) {
		setHeader(PROTOCOL_CONSUME_GROUP, value);
	}
	string getGroupFilter() {
		return getHeader(PROTOCOL_GROUP_FILTER);
	}
	void setGroupFilter(string value) {
		setHeader(PROTOCOL_GROUP_FILTER, value);
	}
	
	string getGroupStartCopy() {
		return getHeader(PROTOCOL_GROUP_START_COPY);
	}
	void setGroupStartCopy(string value) {
		setHeader(PROTOCOL_GROUP_START_COPY, value);
	}
	
	string getGroupStartMsgId() {
		return getHeader(PROTOCOL_GROUP_START_MSGID);
	}
	void setGroupStartMsgId(string value) {
		setHeader(PROTOCOL_GROUP_START_MSGID, value);
	}

	int64_t getGroupStartOffset() {
		return getNumber<int64_t>(PROTOCOL_GROUP_START_OFFSET); 
	} 
	void setGroupStartOffset(int64_t value = -1) {
		setNumber(PROTOCOL_GROUP_START_OFFSET, value); 
	}

	int64_t getGroupStartTime() {
		return getNumber<int64_t>(PROTOCOL_GROUP_START_TIME); 
	}
	void setGroupStartTime(int64_t value = -1) {
		setNumber(PROTOCOL_GROUP_START_TIME, value); 
	}

	int getGroupMask() {
		return getNumber<int>(PROTOCOL_GROUP_MASK); 
	} 
	void setGroupMask(int value = -1) {
		setNumber(PROTOCOL_GROUP_MASK, value); 
	}

	int getConsumeWindow() {
		return getNumber<int>(PROTOCOL_CONSUME_WINDOW); 
	}
	void setConsumeWindow(int value = -1) {
		setNumber(PROTOCOL_CONSUME_WINDOW, value);
	}   

	string getSender() {
		return getHeader(PROTOCOL_SENDER);
	}
	void setSender(string value) {
		setHeader(PROTOCOL_SENDER, value);
	}
	string getRecver() {
		return getHeader(PROTOCOL_RECVER);
	}
	void setRecver(string value) {
		setHeader(PROTOCOL_RECVER, value);
	}
	string getToken() {
		return getHeader(PROTOCOL_TOKEN);
	}
	void setToken(string value) {
		setHeader(PROTOCOL_TOKEN, value);
	}
	string getId() {
		return getHeader(PROTOCOL_ID);
	}
	void setId(string value) {
		setHeader(PROTOCOL_ID, value);
	}
	string getOriginId() {
		return getHeader(PROTOCOL_ORIGIN_ID);
	}
	void setOriginId(string value) {
		setHeader(PROTOCOL_ORIGIN_ID, value);
	}
	string getOriginStatus() {
		return getHeader(PROTOCOL_ORIGIN_STATUS);
	}
	void setOriginStatus(string value) {
		setHeader(PROTOCOL_ORIGIN_STATUS, value);
	}
	string getOriginUrl() {
		return getHeader(PROTOCOL_ORIGIN_URL);
	}
	void setOriginUrl(string value) {
		setHeader(PROTOCOL_ORIGIN_URL, value);
	} 

	bool isAck() {
		string value = getHeader(PROTOCOL_ACK);
		return (value == "true" || value == "True" || value == "1"); 
	}

	void setAck(bool value) {
		setNumber(PROTOCOL_ACK, value?1:0);
	}

	int getTopicMask() {
		return getNumber<int>(PROTOCOL_TOPIC_MASK); 
	} 
	void setTopicMask(int value=-1) {
		setNumber(PROTOCOL_TOPIC_MASK, value); 
	}

	string getHeader(string key, string defaultValue = "") {
		string res = header[key];  
		if (res == "") return defaultValue;
		return res;
	}

	void setHeader(string key, string value) {
		if (value == "") return;
		header[key] = value;
	}

	void removeHeader(string key) {
		header.erase(key);
	}

private:
	template<typename T>
	T getNumber(char* key) {
		std::string value = getHeader(key);
		if (value == "") return -1;
		std::stringstream ss(value);
		T number;
		ss >> number;
		return number;
	}

	template<typename T>
	void setNumber(char* key, T value) {
		if (value < 0) return; 
		setHeader(key, std::to_string(value));
	} 

private:
	void* body = 0;
	int bodyLength = 0;
public:
	string status; //should be integer
	string method = "GET";
	string url = "/";
	map<string, string> header;  
	

public:
	Message() {

	}
	Message(Message& msg) {
		this->status = msg.status;
		this->url = msg.url;
		this->method = msg.method;
		this->header = msg.header;
		this->setBody(msg.body, msg.bodyLength);
	}
	~Message() {
		if (this->body) {
			delete this->body;
			this->body = 0;
			this->bodyLength = 0;
		}
	}

	void setBody(string& body) {
		setBody((void*) body.c_str(), body.size());
	}
	void setBody(void* body, int bodyLength) {
		if (this->body) {
			delete this->body;
		} 
		this->bodyLength = bodyLength;
		this->body = new char[this->bodyLength+1];
		memcpy(this->body, body, bodyLength);
		((char*)this->body)[this->bodyLength] = 0; // make it char* compatible
	}

	void setBody(char* body) {
		setBody(body, strlen(body));
	}

	void setJsonBody(string body) {
		header["content-type"] = "application/json";
		setBody((char*)body.c_str(), body.size());
	}   

	string getBodyString() const{
		string res;
		return res.assign((char*)body, (char*)body + bodyLength); 
	}  

	void* getBody() const {
		return (char*)body;
	}  

	int getBodyLength() const {
		return this->bodyLength;
	}

	void print() {
		ByteBuffer buf;
		encode(buf);
		buf.flip();
		buf.print();
		printf("\n");
	}

	void encode(ByteBuffer& buf) { 
		if (status != "") {
			string desc = HttpStatusTable()[status];
			if (desc == "") {
				desc = "Unknown Status";
			} 
			char data[256];
			snprintf(data, sizeof(data), "HTTP/1.1 %s %s\r\n", status.c_str(), desc.c_str()); 
			buf.put(data);
		} else { 
			char data[256];
			snprintf(data, sizeof(data), "%s %s HTTP/1.1\r\n", method.c_str(), url.c_str()); 
			buf.put(data);
		}

		for (map<string, string>::iterator iter = header.begin(); iter != header.end(); iter++) {
			string key = iter->first;
			string val = iter->second;
			if (key == "content-length") continue;
			buf.putKeyValue((char*)key.c_str(), (char*)val.c_str()); 
		} 
		 
		char len[100];
		snprintf(len, sizeof(len), "%d", bodyLength);
		buf.putKeyValue("content-length", len);

		buf.put("\r\n");
		if (bodyLength > 0) {
			buf.put(body, bodyLength);
		} 
	}  

    inline static Message* decode(ByteBuffer& buf) {
		buf.mark();

		Message* msg = new Message(); 

		int len = findHeadLength(buf);
		if (len < 0) return false;
		char* headerStr = new char[len + 1];
		strncpy(headerStr, buf.begin(), len);
		headerStr[len] = '\0';
		bool ok = parseHead(headerStr, msg);
		delete headerStr;
		if (!ok) {
			buf.reset();
			delete msg;
			return NULL;
		}

		buf.drain(len);
		string contentLen = msg->header["content-length"];
		if (contentLen == "") {//no content-length, treat as message without body
			return msg;
		} 
		int nBody = atoi(contentLen.c_str());
		if (nBody > buf.remaining()) {
			buf.reset();
			delete msg;
			return NULL;
		}  

		msg->body = new unsigned char[nBody];
		msg->bodyLength = nBody;
		buf.get((char*)msg->body, nBody);  
		return msg;
	}

private:
	inline static int findHeadLength(ByteBuffer& buf) {
		char* begin = buf.begin();
		char* p = begin;
		char* end = buf.end();
		while (p + 3 < end) {
			if (*(p + 0) == '\r' && *(p + 1) == '\n' && *(p + 2) == '\r' && *(p + 3) == '\n') {
				return p + 4 - begin;
			}
			p++;
		}
		return -1;
	}

    inline static bool parseHead(char* buf, Message* msg) {   
		char* headCtx;
		char* p = strtok_s(buf, "\r\n", &headCtx);
		if (!p) { 
			return false;
		} 

		char* metaCtx;
		char* m = strtok_s(p, " ", &metaCtx);
		if (cmpIgnoreCase(m, "HTTP", 4)) {
			msg->status = strtok_s(NULL, " ", &metaCtx);
		} else {
			msg->url = strtok_s(NULL, " ", &metaCtx);
		}

		p = strtok_s(NULL, "\r\n", &headCtx);
		while (p) {
			char* d = strchr(p, ':');
			if (d) {//ignore not key value
				char* key = strdupTrimed(p, d - p);
				char* val = strdupTrimed(d + 1, p + strlen(p) - d - 1);
				msg->header[string(key)] = string(val);
				free(key);
				free(val);
			}
			p = strtok_s(NULL, "\r\n", &headCtx);
		}
		return true;
	}  

	inline static bool cmpIgnoreCase(char* s1, char* s2, int n) {
		if (s1 == s2) return true;
		if (s1 == NULL) return false;
		if (s2 == NULL) return false;

		s1 = _strdup(s1);
		s2 = _strdup(s2);
		for (int i = 0; i < strlen(s1); i++) s1[i] = toupper(s1[i]);
		for (int i = 0; i < strlen(s2); i++) s2[i] = toupper(s2[i]);

		int res = strncmp(s1, s2, n);
		free(s1);
		free(s2);
		return res == 0;
	}

	inline static char* strdupTrimed(char* str, int n) {
		char* p0 = str;
		char* p1 = str + n - 1;
		char* res;
		int len;
		while (*p0 == ' ' && p0<(str + n)) p0++;
		while (*p1 == ' ' && p1>str) p1--;
		len = p1 - p0 + 1;
		if (len<1) {
			return _strdup("");
		}
		res = (char*)malloc(len + 1);
		strncpy(res, p0, len);
		res[len] = '\0';
		return res;
	}

	static map<string, string>& HttpStatusTable() {
		static bool init = false;
		static map<string, string> table;
		if (!init) {
			init = true;

			table["200"] = "OK";
			table["201"] = "Created";
			table["202"] = "Accepted";
			table["204"] = "No Content";
			table["206"] = "Partial Content";
			table["301"] = "Moved Permanently";
			table["304"] = "Not Modified";
			table["400"] = "Bad Request";
			table["401"] = "Unauthorized";
			table["403"] = "Forbidden";
			table["404"] = "Not Found";
			table["405"] = "Method Not Allowed";
			table["416"] = "Requested Range Not Satisfiable";
			table["500"] = "Internal Server Error";
		}
		return table;
	}
};
 


#endif