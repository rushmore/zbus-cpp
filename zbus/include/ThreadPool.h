#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "Platform.h"
#include <vector>
#include <queue> 
#include <thread>
#include <mutex>
#include <condition_variable>   

namespace zbus {  
	class ZBUS_API ThreadPool {
	public:
		ThreadPool(int threads);
		void submit(std::function<void()> task); 
		~ThreadPool(); 
	private:
		std::vector<std::thread*> workers;
		std::queue<std::function<void()>> tasks;
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};
}
 
#endif