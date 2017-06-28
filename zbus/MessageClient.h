#ifndef __ZBUS_MESSAGE_CLIENT_H__
#define __ZBUS_MESSAGE_CLIENT_H__  
 
#include "Platform.h"
#include "Message.h"
#include "Logger.h"
#include "Buffer.h"

using namespace std;

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

#define ERR_NET_UNKNOWN_HOST        -86  /**< Failed to get an IP address for the given hostname. */
#define ERR_NET_SOCKET_FAILED       -66  /**< Failed to open a socket. */
#define ERR_NET_CONNECT_FAILED      -68  /**< The connection to the given server / port failed. */ 
#define ERR_NET_RECV_FAILED         -76  /**< Reading information from the socket failed. */
#define ERR_NET_SEND_FAILED         -78  /**< Sending information through the socket failed. */
#define ERR_NET_CONN_RESET          -80  /**< Connection was reset by peer. */
#define ERR_NET_WANT_READ           -82  /**< Connection requires a read call. */
#define ERR_NET_WANT_WRITE          -84  /**< Connection requires a write call. */ 


#ifdef __WINDOWS__
#include <objbase.h>
#else
#include <uuid/uuid.h>
typedef struct _GUID
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID, UUID;

#endif

static void gen_uuid(char* buf, int len) {
	UUID uuid;
#ifdef __WINDOWS__
	CoCreateGuid(&uuid);
#else
	uuid_generate((char*)&uuid);
#endif

	snprintf(buf, len, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uuid.Data1, uuid.Data2, uuid.Data3,
		uuid.Data4[0], uuid.Data4[1],
		uuid.Data4[2], uuid.Data4[3],
		uuid.Data4[4], uuid.Data4[5],
		uuid.Data4[6], uuid.Data4[7]);
}


#ifdef __WINDOWS__
static int wsa_init_done = 0;
#endif
 
/*
* Set the socket blocking or non-blocking
*/
inline static int net_set_block(int fd)
{
#ifdef __WINDOWS__
	u_long n = 0;
	return(ioctlsocket(fd, FIONBIO, &n));
#else
	return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK));
#endif
}

inline static int net_set_nonblock(int fd)
{
#ifdef __WINDOWS__
	u_long n = 1;
	return(ioctlsocket(fd, FIONBIO, &n));
#else
	return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
#endif
}

/*
* Prepare for using the sockets interface
*/
inline static int net_prepare(void)
{
#ifdef __WINDOWS__ 
	if (wsa_init_done == 0)
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) == SOCKET_ERROR)
			return(ERR_NET_SOCKET_FAILED);

		wsa_init_done = 1;
	}
#else
	signal(SIGPIPE, SIG_IGN);
#endif
	return(0);
}


inline static void net_close(int fd){
	shutdown(fd, 2);
#ifdef __WINDOWS__ 
	closesocket(fd);
#else
	close(fd);
#endif
}


#ifdef __WINDOWS__
/*
* Check if the requested operation would be blocking on a non-blocking socket
* and thus 'failed' with a negative return value.
*/
inline static int net_would_block(int fd){
	return(WSAGetLastError() == WSAEWOULDBLOCK);
}
#else
/*
* Check if the requested operation would be blocking on a non-blocking socket
* and thus 'failed' with a negative return value.
*
* Note: on a blocking socket this function always returns 0!
*/
inline static int net_would_block(int fd){
	/*
	* Never return 'WOULD BLOCK' on a non-blocking socket
	*/
	if ((fcntl(fd, F_GETFL) & O_NONBLOCK) != O_NONBLOCK)
		return(0);

	switch (errno)
	{
#if defined EAGAIN
	case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
	case EWOULDBLOCK:
#endif
		return(1);
	}
	return(0);
}
#endif


inline static int net_set_timeout(int fd, int64_t timeout) {
	int rc = -1;
#ifdef __UNIX__
	struct timeval tv;
	tv.tv_sec = (long)(timeout / 1000);
	tv.tv_usec = timeout % 1000 * 10000;
	rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif

#ifdef __WINDOWS__
	rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#endif 
	return rc;
}


inline static int net_connect(int *fd, const char *host, int port){
	int ret;
	struct addrinfo hints, *addr_list, *cur;
	char port_str[6];

	if ((ret = net_prepare()) != 0)
		return(ret);

	/* getaddrinfo expects port as a string */
	memset(port_str, 0, sizeof(port_str));
	snprintf(port_str, sizeof(port_str), "%d", port);

	/* Do name resolution with both IPv6 and IPv4, but only TCP */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(host, port_str, &hints, &addr_list) != 0)
		return(ERR_NET_UNKNOWN_HOST);

	/* Try the sockaddrs until a connection succeeds */
	ret = ERR_NET_UNKNOWN_HOST;
	for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
		*fd = (int)socket(cur->ai_family, cur->ai_socktype,
			cur->ai_protocol);
		if (*fd < 0) {
			ret = ERR_NET_SOCKET_FAILED;
			continue;
		}
		if (connect(*fd, cur->ai_addr, cur->ai_addrlen) == 0) {
			ret = 0;
			break;
		}

#ifdef __WINDOWS__ 
		closesocket(*fd);
#else
		close(*fd);
#endif 
		*fd = -1;
		ret = ERR_NET_CONNECT_FAILED;
	}

	freeaddrinfo(addr_list);

	return(ret);
}

 
inline static int net_recv(int fd, unsigned char *buf, size_t len){
	
#ifdef __WINDOWS__
	int ret = recv(fd, (char*)buf, (int)len, 0);
	if (ret < 0)
	{
		if (net_would_block(fd) != 0)
			return(ERR_NET_WANT_READ);
		if (WSAGetLastError() == WSAECONNRESET)
			return(ERR_NET_CONN_RESET);
		return(ERR_NET_RECV_FAILED);
	}
	return(ret);
#else
	int ret = read(fd, buf, len);

	if (ret < 0)
	{
		if (net_would_block(fd) != 0)
			return(ERR_NET_WANT_READ);
		if (errno == EPIPE || errno == ECONNRESET)
			return(ERR_NET_CONN_RESET);

		if (errno == EINTR)
			return(ERR_NET_WANT_READ);
		
		return(ERR_NET_RECV_FAILED);
	}
	return(ret);
#endif  
	
}
 

inline static int net_send(int fd, const unsigned char *buf, size_t len){
#ifdef __WINDOWS__
	int ret = send(fd, (char*)buf, (int)len, 0);
	if (ret < 0)
	{
		if (net_would_block(fd) != 0)
			return(ERR_NET_WANT_WRITE);
		if (WSAGetLastError() == WSAECONNRESET)
			return(ERR_NET_CONN_RESET);
		return(ERR_NET_SEND_FAILED);
	}
	return ret;
#else
	int ret = write(fd, (char*)buf, (int)len, 0);
	if (ret < 0)
	{
		if (net_would_block(fd) != 0)
			return(ERR_NET_WANT_WRITE);
		if (errno == EPIPE || errno == ECONNRESET)
			return(ERR_NET_CONN_RESET);

		if (errno == EINTR)
			return(ERR_NET_WANT_WRITE);
		return(ERR_NET_SEND_FAILED);
	}
	return ret;
#endif 
}



class ZBUS_API MessageClient { 
public:    
	MessageClient(string address, bool sslEnabled = false, string sslCertFile = "") :
		address(address),
		sslEnabed(sslEnabed),
		sslCertFile(sslCertFile),
		socket(-1)
	{
		logger = Logger::getLogger();
		readBuffer = new ByteBuffer();
	}

	virtual ~MessageClient() {
		this->close(true);
	}
	
	void connect() {
		if (this->socket != -1) {
			return; //already
		}
		{
			std::unique_lock<std::mutex> lock(mutex); 
			if (this->socket != -1) return;

			resetReadBuffer();
			string address = this->address;
			size_t pos = address.find(':');
			int port = 80;
			char* host = (char*)address.substr(0, pos).c_str();
			if (pos != -1) {
				port = atoi(address.substr(pos + 1).c_str());
			}
			if (logger->isDebugEnabled()) {
				logger->debug("Trying connect to (%s)", address.c_str());
			}
			int ret = net_connect(&this->socket, host, port);
			if (ret != 0) {
				string errMsg = errorMessage(ret);
				throw MqException(errMsg, ret);
			}
			if (ret == 0) {
				if (logger->isDebugEnabled()) {
					logger->debug("Connected to (%s)", address.c_str());
				}
			}  
		}
		if (onConnected) {
			onConnected(this);
		}
	}

	Message* invoke(Message& msg, int timeout = 3000) {
		connect();
		std::unique_lock<std::mutex> lock(mutex);
		sendUnsafe(msg, timeout);
		string msgid = msg.getId();
		return recvUnsafe(msgid.c_str(), timeout);
	}

	void send(Message& msg, int timeout = 3000) {
		connect();
		std::unique_lock<std::mutex> lock(mutex);
		sendUnsafe(msg, timeout);
	}

	Message* recv(const char* msgid = NULL, int timeout = 3000) {
		connect();
		std::unique_lock<std::mutex> lock(mutex);
		return recvUnsafe(msgid, timeout);
	}

	void start(int timeout=10000) {
		std::unique_lock<std::mutex> lock(processMutex);
		if (processThread != NULL) return;
		processThread = new std::thread(&MessageClient::processMessage, this, timeout);
	}

	void join() {
		if (processThread) {
			processThread->join();
		}
	}

	void close(bool stopThread=true) {
		if (stopThread) {
			autoConnect = false;
		}
		if (socket != -1) {
			net_close(socket);
			socket = -1;
		}
		for (auto &kv : msgTable) {
			delete kv.second;
		}
		msgTable.clear();

		if (readBuffer) {
			delete readBuffer;
			readBuffer = NULL;
		}

		if (stopThread && this->processThread) { 
			this->processThread->join();
			delete this->processThread;
			this->processThread = NULL;
		}
	}

private:
	void processMessage(int timeout=10000) {
		while (true) {
			try {
				Message* msg = recv(NULL, timeout); 

				if (onMessage) {
					onMessage(msg, this->contextObject);
				} else {
					delete msg;
				} 
			}
			catch (MqException& e) {
				if (!autoConnect) break;  

				if (e.code == ERR_NET_RECV_FAILED) { //timeout?
					continue;
				}

				logger->error("%d, %s", e.code, e.message.c_str());
				close(false); //no stop of thread
				if (this->onDisconnected) {
					this->onDisconnected();
				}
				std::this_thread::sleep_for(std::chrono::seconds(3)); 
			} 
		}
	}
	void sendUnsafe(Message& msg, int timeout=3000) {
		int ret = net_set_timeout(this->socket, timeout);
		if (ret < 0) {
			string errMsg = errorMessage(ret);
			throw MqException(errMsg, ret);
		}

		if (msg.getId() == "") {
			char uuid[256];
			gen_uuid(uuid, sizeof(uuid));
			msg.setId(uuid);
		}
		ByteBuffer buf;
		msg.encode(buf);
		buf.flip();

		int sent = 0, total = buf.remaining();
		unsigned char* start = (unsigned char*)buf.begin();
		while (sent < total) {
			int ret = net_send(this->socket, start, total - sent);
			if (ret < 0) {  
				string errMsg = errorMessage(ret);
				throw MqException(errMsg, ret); 
			}
			sent += ret;
			start += ret;
		}
		if (logger->isDebugEnabled()) {
			logger->debug((void*)buf.begin(), buf.remaining());
		} 
	}


	Message* recvUnsafe(const char* msgid = NULL, int timeout = 3000) {
		if (msgid) {
			Message* res = msgTable[msgid];
			if (res) {
				msgTable.erase(string(msgid));
				return res;
			}
		}

		int rc = net_set_timeout(this->socket, timeout);
		if (rc < 0) {
			string errMsg = errorMessage(rc);
			throw MqException(errMsg, rc);
		}
		if (logger->isDebugEnabled()) {
			logger->logHead(LOG_DEBUG);
		}
		while (true) {
			unsigned char data[10240];
			int n = net_recv(this->socket, data, sizeof(data));
			if (n < 0) {
				rc = n;
				string errMsg = errorMessage(rc);
				throw MqException(errMsg, rc);
			}
			readBuffer->put((void*)data, n);
			if (logger->isDebugEnabled()) {
				logger->logBody((void*)data, n, LOG_DEBUG);
			}

			ByteBuffer buf(readBuffer); //duplicate, no copy of data
			buf.flip();

			Message* msg = Message::decode(buf);
			if (msg == NULL) {
				continue;
			}

			ByteBuffer* newBuf = new ByteBuffer(buf.begin(), buf.remaining());
			delete this->readBuffer;
			this->readBuffer = newBuf;

			if (msgid == NULL || msg->getId() == msgid) {
				return msg;
			}
			msgTable[msgid] = msg;
		}
	}

private:
	Logger* logger;
private:
	int socket;

	string address;
	bool sslEnabed;

	string sslCertFile;
	map<string, Message*> msgTable; 
	ByteBuffer* readBuffer;
	mutable std::mutex mutex;

public:
	typedef void(*ConnectedHandler)(MessageClient* client);
	typedef void(*DisconnectedHandler)();
	typedef void(*MessageHandler)(Message*, void*); //message need to be deleted in handler

	ConnectedHandler onConnected;
	DisconnectedHandler onDisconnected;
	MessageHandler onMessage;

	void* contextObject;
private:
	bool autoConnect = true;
	std::thread* processThread;
	mutable std::mutex processMutex;

private:
	void resetReadBuffer() {
		if (readBuffer) {
			delete readBuffer;
		}
		readBuffer = new ByteBuffer();
	}

public:
	inline static string errorMessage(int code) {
		map<int, string>& table = NetErrorTable();
		string res = table[code];
		if (res == "") {
			res = "Unknown error";
		}
		return res;
	}
	inline static map<int, string>& NetErrorTable() {
		static bool init = false;
		static map<int, string> table;
		if (!init) {
			init = true;
			table[ERR_NET_UNKNOWN_HOST] = "Failed to get an IP address for the given hostname";
			table[ERR_NET_SOCKET_FAILED] = "Failed to open a socket";
			table[ERR_NET_CONNECT_FAILED] = "The connection to the given server / port failed"; 
			table[ERR_NET_RECV_FAILED] = "Reading information from the socket failed";
			table[ERR_NET_SEND_FAILED] = "Sending information through the socket failed";
			table[ERR_NET_CONN_RESET] = "Connection was reset by peer";
			table[ERR_NET_WANT_READ] = "Connection requires a read call";
			table[ERR_NET_WANT_WRITE] = "Connection requires a write call"; 
		}
		return table;
	}
	
};

#endif