#include "MqClient.h"  
#include <iostream>
using namespace zbus;
using namespace std;


int main_networkbase(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_INFO); 
	Logger* log = Logger::getLogger();

	MessageClient* client = new MessageClient("localhost:15555");
	//client->connect();
	client->start();
	Sleep(1000); 

	delete client;

	system("pause");
	return 0;
}