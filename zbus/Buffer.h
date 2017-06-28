#ifndef __ZBUS_BUFFER_H__
#define __ZBUS_BUFFER_H__   

#include "Platform.h" 
using namespace std;

class ZBUS_API ByteBuffer {
private:
	int mark_ = -1;
	int position = 0;
	int limit_;
	int capacity;
	int ownData = 0;
	char* data = 0;

public: 
	ByteBuffer(int capacity=10240) {
		this->limit_ = this->capacity = capacity;

		ownData = 1;
		data = new char[capacity]; 
	}

	ByteBuffer(char* array, int len) {
		ownData = 1;
		limit_ = capacity = len;
		data = new char[capacity];
		memcpy(data, array, capacity); 
	}

	ByteBuffer(ByteBuffer* buf) {
		this->capacity = buf->capacity;
		this->data = buf->data;
		this->position = buf->position;
		this->limit_ = buf->limit_;
		this->mark_ = buf->mark_;
		this->ownData = 0;
	} 

	~ByteBuffer() {
		if (ownData) {
			delete data;
			data = 0;
		}
	}  

	void mark() {
		this->mark_ = this->position;
	}

	ByteBuffer* flip() {
		this->limit_ = this->position;
		this->position = 0;
		this->mark_ = -1;
		return this;
	} 

	void reset() {
		int m = this->mark_;
		if (m < 0) {
			throw new std::exception("mark not set, reset discard"); 
		}
		this->position = m;
	}  
	int remaining() {
		return this->limit_ - this->position;
	}

	char* begin() { 
		return this->data + this->position;
	}
	char* end() { 
		return this->data + this->limit_;
	} 

	ByteBuffer* limit(int newLimit) {
		if (newLimit>this->capacity || newLimit<0) {
			throw new exception("set new limit error, discarding"); 
		}
		this->limit_ = newLimit;
		if (this->position > this->limit_) this->position = this->limit_;
		if (this->mark_ > this->limit_) this->mark_ = -1;
		return this;
	}

	int drain(int n) {
		if (n <= 0) return 0;

		int res = n;
		int newPos = this->position + n;
		if (newPos>this->limit_) {
			newPos = this->limit_;
			res = newPos - this->position;
		}
		this->position = newPos;
		if (this->mark_ > this->position) this->mark_ = -1;
		return res;
	}

	int copyout(char data[], int len) {
		if (remaining()<len) {
			return -1;
		}
		memcpy(data, this->begin(), len);
		return len;
	}

	int get(char data[], int len) {
		int copyLen = copyout(data, len);
		if (copyLen > 0) {
			drain(len);
		}
		return copyLen;
	}  

	int put(void* data, int len) {
		expandIfNeeded(len);
		memcpy(this->begin(), data, len);
		drain(len);
		return len;
	}

	int put(ByteBuffer* buf) {
		return put(buf->begin(), buf->remaining());
	}

	int put(char* str) {
		return put(str, strlen(str));
	} 

	int putKeyValue(char* key, char* val) {
		int len = 0;
		len += put(key);
		len += put(": ");
		len += put(val);
		len += put("\r\n");
		return len;
	} 

	void print() {
		int len = this->remaining();
		char* data = new char[len + 1];
		memcpy(data, this->begin(), len);
		data[len] = '\0';
		printf("%s", data);
		delete data;
	}

private:
	int expandIfNeeded(int need) {
		if (this->ownData == 0) return -1; //can not expand for duplicated

		int new_cap = this->capacity;
		if (new_cap <= 0) {
			new_cap = 10240; //default 
		}
		int new_size = this->position + need;
		char* new_data;
		
		while (new_size > new_cap) {
			new_cap *= 2;
		}
		if (new_cap == this->capacity) return 0;//nothing changed

		new_data = new char[new_cap];
		memcpy(new_data, this->data, this->capacity);
		delete this->data;
		this->data = new_data;
		this->capacity = new_cap;
		this->limit_ = new_cap;
		return 1;
	}
};


#endif