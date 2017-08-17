#include "ThreadPool.h"
#include <iostream>
using namespace std;
using namespace zbus;
int main_ThreadPool(int argc, char* argv[]) {
	 
	ThreadPool pool(4); 
	static int count = 0;
	for (int i = 0; i < 10; i++) {
		pool.submit([]() {
			std::this_thread::sleep_for(std::chrono::seconds(1+ count++));
			cout << "test ok" << endl;
		});
	} 
	 
	return 0; 
}