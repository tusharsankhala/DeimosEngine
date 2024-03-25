#include "Common/stdafx.h"
#include "Common/Misc/ThreadPool.h"

static ThreadPool g_threadPool;

ThreadPool* GetThreadPool()
{
	return &g_threadPool;
}

#define ENABLE_MULTI_THREADING

ThreadPool::ThreadPool()
{
	m_activeThreads = 0;

#ifdef ENABLE_MULTI_THREADING
	num_threads = std::thread::hardware_concurrency();
	bExiting = false;
	for( int i=0; i < num_threads; ++i )
	{
		pool.push_back( std::thread(&ThreadPool::JobStealerLoop, GetThreadPool() ) );
	}
#endif
}

ThreadPool::~ThreadPool()
{
#ifdef ENABLE_MULTI_THREADING
	bExiting = true;
	condition.notify_all();
	for (int i = 0; i < num_threads; ++i)
	{
		pool[i].join();
	}
#endif
}

void ThreadPool::JobStealerLoop()
{
#ifdef ENABLE_MULTI_THREADING
	while( true )
	{
		Task t;
		{
			std::unique_lock<std::mutex> lock( queue_mutex );

			condition.wait(lock, [this] { return bExiting || (!queue.empty() && (m_activeThreads < num_threads)); });
			if (bExiting == false)
				return;

			++m_activeThreads;

			t = queue.front();
			queue.pop_front();
		}

		t.m_job();
		{
			std::unique_lock<std::mutex> lock( queue_mutex );
			--m_activeThreads;
		}
	}
#endif
}

void ThreadPool::AddJob(std::function<void()> new_job)
{
#ifdef ENABLE_MULTI_THREADING
	if (bExiting == false)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		Task t;
		t.m_job = new_job;

		queue.push_back(t);

		if (m_activeThreads < num_threads)
		{
			condition.notify_one();
		}
	}
#else
	job();
#endif
}
