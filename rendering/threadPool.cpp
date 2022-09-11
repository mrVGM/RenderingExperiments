#include "threadPool.h"

#include <thread>
#include <list>
#include <mutex>
#include <semaphore>

namespace
{
	struct Thread
	{
		bool m_active = false;
		std::binary_semaphore m_semaphore{1};
		rendering::threadPool::Runnable* runnable = nullptr;
		std::thread* m_thread = nullptr;
	};

	std::mutex m_poolMutex;
	std::list<Thread*> m_pool;

	Thread* m_current = nullptr;

	std::binary_semaphore m_startThreadSemaphore{1};

	void run()
	{
		Thread* myThread = m_current;
		m_current = nullptr;

		m_startThreadSemaphore.release();

		while (true) {
			myThread->m_semaphore.acquire();

			myThread->runnable->Run();
			myThread->runnable->Ready();

			myThread->m_active = false;
		}
	}

	Thread* CreateThread()
	{
		m_pool.push_back(new Thread());
		Thread* thread = m_pool.back();

		m_startThreadSemaphore.acquire();
		m_current = thread;

		thread->m_semaphore.acquire();
		thread->m_thread = new std::thread(run);

		return thread;
	}
}

void rendering::threadPool::StartRoutine(Runnable* runnable)
{
	m_poolMutex.lock();
	Thread* thread = nullptr;
	for (std::list<Thread*>::iterator it = m_pool.begin(); it != m_pool.end(); ++it) {
		Thread* cur = *it;
		if (!cur->m_active) {
			thread = cur;
			break;
		}
	}

	if (!thread) {
		thread = CreateThread();
	}

	thread->m_active = true;
	m_poolMutex.unlock();

	thread->runnable = runnable;
	thread->m_semaphore.release();
}
