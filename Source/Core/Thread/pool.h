#pragma once
#include <Core/commdefs.h>
#include "Core/Container/basic.h"
#include "Core/Container/dummy.h"
#include "Core/Thread/thread.h"
#include "Core/Thread/event.h"
#include "Core/Thread/lock.h"

namespace glex
{
	class QueuedWork
	{
	public:
		virtual void DoWork() {};
		virtual void Abort() {};
		virtual ~QueuedWork() {}
	};

	class ThreadPool : private Unmoveable
	{
	private:
		struct PooledThreadContext
		{
			Thread thread;
			Event* hasWorkEvent;
			ThreadPool* owner;
			QueuedWork* work;

			template <typename Fn>
			PooledThreadContext(Fn&& proc, ThreadPriority priority, ThreadPool* pool) :
				thread(std::forward<Fn>(proc), priority), hasWorkEvent(Event::Get(false)), owner(pool), work(nullptr) {}
			~PooledThreadContext() { Event::Release(hasWorkEvent); }
			// Will not be called.
			PooledThreadContext(PooledThreadContext&& rhs) :
				thread(std::move(rhs.thread)), hasWorkEvent(rhs.hasWorkEvent), owner(rhs.owner), work(rhs.work)
			{
				rhs.hasWorkEvent = nullptr;
			}
		};

		Vector<PooledThreadContext> m_threads;
		Vector<PooledThreadContext*> m_freeList;
		Deque<QueuedWork*> m_workQueue;
		Mutex m_lock;
		bool m_isShuttingDown;

		static void ThreadMain(PooledThreadContext& context);

	public:
		ThreadPool(uint32_t numThreads);
		~ThreadPool();
		uint32_t FreeThreadCount() const { return m_freeList.size(); }
		void SubmitWork(QueuedWork* work);
	};
}