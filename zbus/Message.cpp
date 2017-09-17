#include "Message.h"


namespace zbus { 
	 
	void Message::encode(ByteBuffer& buf) {
		if (status != "") {
			std::string desc = HttpStatusTable()[status];
			if (desc == "") {
				desc = "Unknown Status";
			}
			char data[256];
			snprintf(data, sizeof(data), "HTTP/1.1 %s %s\r\n", status.c_str(), desc.c_str());
			buf.put(data);
		}
		else {
			char data[256];
			snprintf(data, sizeof(data), "%s %s HTTP/1.1\r\n", method.c_str(), url.c_str());
			buf.put(data);
		}

		for (std::map<std::string, std::string>::iterator iter = header.begin(); iter != header.end(); iter++) {
			std::string key = iter->first;
			std::string val = iter->second;
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

	Message* Message::decode(ByteBuffer& buf) {
		buf.mark();

		Message* msg = new Message();

		int len = findHeadLength(buf);
		if (len < 0) return false;
		char* headerStr = new char[len + 1];
		strncpy(headerStr, buf.begin(), len);
		headerStr[len] = '\0';
		bool ok = parseHead(headerStr, msg);
		delete[] headerStr;
		if (!ok) {
			buf.reset();
			delete msg;
			return NULL;
		}

		buf.drain(len);
		std::string contentLen = msg->header["content-length"];
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

	int Message::findHeadLength(ByteBuffer& buf) {
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

	bool Message::parseHead(char* buf, Message* msg) {
		char* headCtx;
		char* p = strtok_s(buf, "\r\n", &headCtx);
		if (!p) {
			return false;
		}

		char* metaCtx;
		char* m = strtok_s(p, " ", &metaCtx);
		if (cmpIgnoreCase(m, "HTTP", 4)) {
			msg->status = strtok_s(NULL, " ", &metaCtx);
		}
		else {
			msg->url = strtok_s(NULL, " ", &metaCtx);
		}

		p = strtok_s(NULL, "\r\n", &headCtx);
		while (p) {
			char* d = strchr(p, ':');
			if (d) {//ignore not key value
				char* key = strdupTrimed(p, d - p);
				char* val = strdupTrimed(d + 1, p + strlen(p) - d - 1);
				msg->header[std::string(key)] = std::string(val);
				free(key);
				free(val);
			}
			p = strtok_s(NULL, "\r\n", &headCtx);
		}
		return true;
	}

	bool Message::cmpIgnoreCase(char* s1, char* s2, int n) {
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

	char* Message::strdupTrimed(char* str, int n) {
		char* p0 = str;
		char* p1 = str + n - 1;
		char* res;
		int len;
		while (*p0 == ' ' && p0 < (str + n)) p0++;
		while (*p1 == ' ' && p1 > str) p1--;
		len = p1 - p0 + 1;
		if (len < 1) {
			return _strdup("");
		}
		res = (char*)malloc(len + 1);
		strncpy(res, p0, len);
		res[len] = '\0';
		return res;
	}

	std::map<std::string, std::string>& Message::HttpStatusTable() {
		static bool init = false;
		static std::map<std::string, std::string> table;
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
}//namespace