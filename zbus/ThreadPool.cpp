#include "ThreadPool.h"


namespace zbus { 

	ThreadPool::ThreadPool(int threads) : stop(false) {
		for (int i = 0; i < threads; i++) {
			std::thread* worker = new std::thread([this] {
				for (;;) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty()) return;
						task = this->tasks.front();
						this->tasks.pop();
					}
					try {
						task();
					}
					catch (std::exception& e) {
						//ignore
					}
				}
			});
			workers.push_back(worker);
		}
	}

	void ThreadPool::submit(std::function<void()> task) {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.push(task);
		}

		condition.notify_one();
	}

	ThreadPool::~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread *worker : workers) {
			worker->join();
			delete worker;
		}
	} 
}