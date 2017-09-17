#ifndef __ZBUS_MESSAGE_H__
#define __ZBUS_MESSAGE_H__   

#include "Platform.h"
#include "Protocol.h"
#include "Buffer.h" 
#include <sstream> 

namespace zbus {

	class ZBUS_API Message {
	public:
		std::string getCmd() {
			return getHeader(PROTOCOL_CMD);
		}
		void setCmd(std::string value) {
			setHeader(PROTOCOL_CMD, value);
		}
		std::string getTopic() {
			return getHeader(PROTOCOL_TOPIC);
		}
		void setTopic(std::string value) {
			setHeader(PROTOCOL_TOPIC, value);
		}
		std::string getConsumeGroup() {
			return getHeader(PROTOCOL_CONSUME_GROUP);
		}
		void setConsumeGroup(std::string value) {
			setHeader(PROTOCOL_CONSUME_GROUP, value);
		}
		std::string getGroupFilter() {
			return getHeader(PROTOCOL_GROUP_FILTER);
		}
		void setGroupFilter(std::string value) {
			setHeader(PROTOCOL_GROUP_FILTER, value);
		}

		std::string getGroupStartCopy() {
			return getHeader(PROTOCOL_GROUP_START_COPY);
		}
		void setGroupStartCopy(std::string value) {
			setHeader(PROTOCOL_GROUP_START_COPY, value);
		}

		std::string getGroupStartMsgId() {
			return getHeader(PROTOCOL_GROUP_START_MSGID);
		}
		void setGroupStartMsgId(std::string value) {
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

		std::string getSender() {
			return getHeader(PROTOCOL_SENDER);
		}
		void setSender(std::string value) {
			setHeader(PROTOCOL_SENDER, value);
		}
		std::string getRecver() {
			return getHeader(PROTOCOL_RECVER);
		}
		void setRecver(std::string value) {
			setHeader(PROTOCOL_RECVER, value);
		}
		std::string getToken() {
			return getHeader(PROTOCOL_TOKEN);
		}
		void setToken(std::string value) {
			setHeader(PROTOCOL_TOKEN, value);
		}
		std::string getId() {
			return getHeader(PROTOCOL_ID);
		}
		void setId(std::string value) {
			setHeader(PROTOCOL_ID, value);
		}
		std::string getOriginId() {
			return getHeader(PROTOCOL_ORIGIN_ID);
		}
		void setOriginId(std::string value) {
			setHeader(PROTOCOL_ORIGIN_ID, value);
		}
		std::string getOriginStatus() {
			return getHeader(PROTOCOL_ORIGIN_STATUS);
		}
		void setOriginStatus(std::string value) {
			setHeader(PROTOCOL_ORIGIN_STATUS, value);
		}
		std::string getOriginUrl() {
			return getHeader(PROTOCOL_ORIGIN_URL);
		}
		void setOriginUrl(std::string value) {
			setHeader(PROTOCOL_ORIGIN_URL, value);
		}

		bool isAck() {
			std::string value = getHeader(PROTOCOL_ACK);
			return (value == "true" || value == "True" || value == "1");
		}

		void setAck(bool value) {
			setNumber(PROTOCOL_ACK, value ? 1 : 0);
		}

		int getTopicMask() {
			return getNumber<int>(PROTOCOL_TOPIC_MASK);
		}
		void setTopicMask(int value = -1) {
			setNumber(PROTOCOL_TOPIC_MASK, value);
		}

		std::string getHeader(std::string key, std::string defaultValue = "") {
			std::string res = header[key];
			if (res == "") return defaultValue;
			return res;
		}

		void setHeader(std::string key, std::string value) {
			if (value == "") return;
			header[key] = value;
		}

		void removeHeader(std::string key) {
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
		std::string status; //should be integer
		std::string method = "GET";
		std::string url = "/";
		std::map<std::string, std::string> header;


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
				delete[] this->body;
				this->body = 0;
				this->bodyLength = 0;
			}
		}

		void setBody(std::string& body) {
			setBody((void*)body.c_str(), body.size());
		}
		void setBody(void* body, int bodyLength) {
			if (this->body) {
				delete[] this->body;
			}
			this->bodyLength = bodyLength;
			this->body = new char[this->bodyLength + 1];
			memcpy(this->body, body, bodyLength);
			((char*)this->body)[this->bodyLength] = 0; // make it char* compatible
		}

		void setBody(char* body) {
			setBody(body, strlen(body));
		}

		void setJsonBody(std::string body) {
			header["content-type"] = "application/json";
			setBody((char*)body.c_str(), body.size());
		}

		std::string getBodyString() const {
			std::string res;
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

		void encode(ByteBuffer& buf); 
		static Message* decode(ByteBuffer& buf);
	private:
		static int findHeadLength(ByteBuffer& buf); 
		static bool parseHead(char* buf, Message* msg);
		static bool cmpIgnoreCase(char* s1, char* s2, int n);
		static char* strdupTrimed(char* str, int n);

		static std::map<std::string, std::string>& HttpStatusTable();
	};


}//namespace
#endif