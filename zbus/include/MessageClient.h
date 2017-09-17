#ifndef __ZBUS_MESSAGE_CLIENT_H__
#define __ZBUS_MESSAGE_CLIENT_H__  

#include "Platform.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h" 

#include <thread>
#include <condition_variable>
#include <mutex> 
#include <chrono> 


#define ERR_NET_UNKNOWN_HOST        -86  /**< Failed to get an IP address for the given hostname. */
#define ERR_NET_SOCKET_FAILED       -66  /**< Failed to open a socket. */
#define ERR_NET_CONNECT_FAILED      -68  /**< The connection to the given server / port failed. */ 
#define ERR_NET_RECV_FAILED         -76  /**< Reading information from the socket failed. */
#define ERR_NET_SEND_FAILED         -78  /**< Sending information through the socket failed. */
#define ERR_NET_CONN_RESET          -80  /**< Connection was reset by peer. */
#define ERR_NET_WANT_READ           -82  /**< Connection requires a read call. */
#define ERR_NET_WANT_WRITE          -84  /**< Connection requires a write call. */ 

//////////////////////////////////////////////////////////////////////////////////////////
namespace zbus {

	struct TimerKiller {
		bool waitFor(std::chrono::milliseconds const& time) {
			std::unique_lock<std::mutex> lock(m);
			return !cv.wait_for(lock, time, [&] {return terminate; });
		}
		void kill() {
			std::unique_lock<std::mutex> lock(m);
			terminate = true;
			cv.notify_all();
		}
	private:
		std::condition_variable cv;
		std::mutex m;
		bool terminate = false;
	};


	class ZBUS_API MessageClient {
	public:
		ServerAddress serverAddress;
		std::function<void(MessageClient*)> onConnected;
		std::function<void()> onDisconnected;
		std::function<void(Message*)> onMessage;

		int reconnectInterval = 3000; //3s, in milliseconds
		int heartbeatInterval = 60000; //60s, in milliseconds

	public: 
		MessageClient(std::string address, bool sslEnabled = false, std::string sslCertFile = ""); 
		virtual ~MessageClient();

		bool active(); 
		void connect();
		Message* invoke(Message& msg, int timeout = 3000); 
		void send(Message& msg, int timeout = 3000); 
		Message* recv(const char* msgid = NULL, int timeout = 3000); 
		void start(int timeout = 60000); 
		void join(); 
		void close();

	public:
		static std::string errorMessage(int code);
		static std::map<int, std::string>& NetErrorTable();

	private:
		void startHeartbeat(); 
		void heartbeat();
		void closeSocket(); 
		void sendUnsafe(Message& msg, int timeout = 3000); 
		Message* recvUnsafe(const char* msgid = NULL, int timeout = 3000); 
		void processMessage(int timeout = 60000); 
		void resetReadBuffer();
	private:
		Logger* logger; 
		int socket;
		std::string address;
		bool sslEnabed;

		std::string sslCertFile;
		std::map<std::string, Message*> msgTable;
		ByteBuffer* readBuffer;
		mutable std::mutex connectMutex;
		mutable std::mutex readMutex;
		mutable std::mutex writeMutex;  

	private:
		bool autoConnect = true;
		bool termintated = false; 
		std::thread* processThread;
		std::thread* heartbeatThread;
		TimerKiller* processTimer = NULL;
		TimerKiller* heartbeatTimer = NULL;  
	};

}//namespace

#endif