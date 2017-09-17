#ifndef __ZBUS_PRODUCER_H__
#define __ZBUS_PRODUCER_H__  
 
#include "MqAdmin.h" 

namespace zbus {

	class ZBUS_API Producer : public MqAdmin {
	protected:
		ServerSelector produceSelector;
	public:
		Producer(Broker* broker);
		Message produce(Message& msg, int timeout = 3000, ServerSelector selector = NULL); 
		/**
		Need event loop facility to make the async work smoothingly, such as libuv from NodeJS
		*/
		void produceAsync(Message& msg, int timeout = 3000, ServerSelector selector = NULL);
	};

}//namespace
  
#endif