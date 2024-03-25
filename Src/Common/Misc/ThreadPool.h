#pragma once

#include <functional>
#include <thread>
#include <deque>
#include <condition_variable>


struct Task
{
	std::function<void()>	m_job;
	std::vector<Task *>		m_childTask;
};

class ThreadPool
{
public:
			ThreadPool();
			~ThreadPool();
	void	JobStealerLoop();
	void	AddJob( std::function<void()> New_job );

private:
	bool						bExiting;
	int							num_threads;
	int							m_activeThreads = 0;
	std::vector<std::thread>	pool;
	std::deque<Task>			queue;
	std::condition_variable		condition;
	std::mutex					queue_mutex;
};


ThreadPool*						GetThreadPool();