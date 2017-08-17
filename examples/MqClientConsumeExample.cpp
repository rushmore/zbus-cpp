#include "MqClient.h"  
using namespace zbus;
using namespace std;


int main_mqclient_consume(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO); 
	Logger* log = Logger::getLogger();

	MqClient client("localhost:15555");
	
	while (true) {
		Message* msg = client.consume("MyTopic");
		if (msg == NULL) {
			break;
		}
		delete msg;
	} 


	system("pause");
	return 0;
}