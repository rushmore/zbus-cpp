#include "MqClient.h" 
using namespace zbus;
using namespace std;

int main_msgclient(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO);

	MessageClient client("localhost:15556"); 
	client.start();
	Sleep(100);
	client.close();
	return 0;
}