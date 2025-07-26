#include "Core/Thread/pool.h"
#include "Core/log.h"
#include <Windows.h>

using namespace glex;

void ThreadPool::ThreadMain(ThreadPool::PooledThreadContext& context)
{
	for (;;)
	{
		context.hasWorkEvent->Wait();
		if (context.work == nullptr)
			break;
		Mutex& lock = context.owner->m_lock;
		auto& workQueue = context.owner->m_workQueue;
		for (;;)
		{
			context.work->DoWork();
			Mem::Delete(context.work);
			lock.Lock();
			if (workQueue.empty())
				break;
			context.work = std::move(workQueue.front());
			workQueue.pop_front();
			lock.Unlock();
		}
		if (context.owner->m_isShuttingDown)
			break;
		context.work = nullptr;
		context.owner->m_freeList.emplace_back(&context);
		lock.Unlock();
	}
}

ThreadPool::ThreadPool(uint32_t numThreads) : m_lock(512), m_isShuttingDown(false)
{
	m_threads.reserve(numThreads);
	m_freeList.reserve(numThreads);
	for (uint32_t i = 0; i < numThreads; i++)
	{
		PooledThreadContext& context = m_threads.emplace_back([&context = m_threads[i]]() { ThreadMain(context); }, ThreadPriority::Low, this);
		m_freeList.emplace_back(&context);
		if (!m_threads[i].thread.IsValid())
			Logger::Fatal("Cannot create thread for thread pool.");
		context.thread.Resume();
	}
}

ThreadPool::~ThreadPool()
{
	m_isShuttingDown = true;
	m_lock.Lock();
	for (QueuedWork* work : m_workQueue)
	{
		work->Abort();
		Mem::Delete(work);
	}
	m_workQueue.clear();
	m_lock.Unlock();
	for (auto& context : m_threads)
	{
		context.hasWorkEvent->Set();
		context.thread.Wait();
	}
}

void ThreadPool::SubmitWork(QueuedWork* fn)
{
	ScopedLock lock(m_lock);
	if (m_freeList.size() != 0)
	{
		PooledThreadContext& context = *m_freeList.back();
		m_freeList.pop_back();
		context.work = std::move(fn);
		context.hasWorkEvent->Set();
	}
	else
		m_workQueue.push_back(std::move(fn));
}