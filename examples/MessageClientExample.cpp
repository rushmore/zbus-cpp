#include "MqClient.h" 


int main_MessageClient(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_DEBUG);

	MessageClient client("localhost:15555");
	client.connect();

	for (int i = 0; i<100; i++) {
		Message msg;
		msg.setCmd("server");
		msg.setBody("hello world");

		client.send(msg);
		string msgid = msg.getId();
		Message* res = client.recv(msgid.c_str());
		if (res) {
			res->print();
			delete res;
		}
	}
	system("pause");
	return 0;
}