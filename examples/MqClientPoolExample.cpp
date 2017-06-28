#include "MqClient.h"  


int main_MqClientPool(int argc, char* argv[]) {  
	Logger::configDefaultLogger(0, LOG_DEBUG); 
	Logger* log = Logger::getLogger();

	{
		MqClientPool pool("localhost:15555"); 
		MqClient* client = pool.borrowClient(); 
		client->queryTracker(); 
		pool.returnClient(client);
	}

	system("pause");
	return 0;
}