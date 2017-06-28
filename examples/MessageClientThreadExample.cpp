#include "MqClient.h" 



int main_MessageClientThread(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);

	MessageClient client("localhost:15555");
	
	client.onMessage = [](Message* msg, void* ctx) {
		msg->print();
		delete msg;
	};
	
	client.onConnected = [](MessageClient* client) {
		Message msg;
		msg.setCmd("track_sub");
		client->send(msg);
	};

	client.start();

	client.join();
	system("pause");
	return 0;
}