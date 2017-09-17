#ifndef __ZBUS_BUFFER_H__
#define __ZBUS_BUFFER_H__   

#include "Platform.h"  
namespace zbus {

	class ZBUS_API ByteBuffer {
	private:
		int mark_ = -1;
		int position = 0;
		int limit_;
		int capacity;
		int ownData = 0;
		char* data = 0;

	public:
		ByteBuffer(int capacity = 10240); 
		ByteBuffer(char* array, int len); 
		ByteBuffer(ByteBuffer* buf); 
		~ByteBuffer();

		void mark(); 
		ByteBuffer* flip(); 
		void reset();
		int remaining(); 
		char* begin();
		char* end(); 
		ByteBuffer* limit(int newLimit); 
		int drain(int n);
		int copyout(char data[], int len); 
		int get(char data[], int len); 
		int put(void* data, int len); 
		int put(ByteBuffer* buf); 
		int put(char* str); 
		int putKeyValue(char* key, char* val); 
		void print();

	private:
		int expandIfNeeded(int need);
	}; 
}

#endif