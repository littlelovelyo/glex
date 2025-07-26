#pragma once
#include "Core/Memory/mem.h"
#include "Core/Memory/smart_ptr.h"
#include "Core/Thread/event.h"
#include "Core/Thread/pool.h"
#include "Core/Container/optional.h"

namespace glex
{
	template <typename Ret>
	class Future
	{
	protected:
		Event* m_event;
		Ret m_value;

	public:
		Future() : m_event(Event::Get(true)) {}
		virtual ~Future() { Event::Release(m_event); }
		Ret& Await() { m_event->Wait(); return m_value; }

		template <typename R>
		void SetValue(R&& value)
		{
			m_value = std::forward<R>(value);
			m_event->Set();
		}
	};

	template <>
	class Future<void>
	{
	protected:
		Event* m_event;

	public:
		Future() : m_event(Event::Get(true)) {};
		virtual ~Future() { Event::Release(m_event); }
		void Await() { m_event->Wait(); }
		void SetValue() { m_event->Set(); }
	};

	template <typename Ret>
	class Task
	{
	private:
		SharedPtr<Future<Ret>> m_future;

	public:
		Task(SharedPtr<Future<Ret>> const& future) : m_future(future) {}
		Ret const& Await() const { return m_future->Await(); }
	};

	class Async : private StaticClass
	{
	private:
		inline static Optional<ThreadPool> s_threadPool;

		template <typename Ret, typename Func, bool TRIVIAL>
		class WorkInternal : public QueuedWork
		{
		private:
			SharedPtr<Future<Ret>> m_future;
			Func m_function;

		public:
			template <typename R>
			WorkInternal(SharedPtr<Future<Ret>> const& future, R&& func) : m_future(future), m_function(std::forward<R>(func)) {}
			virtual void DoWork() override { m_future->SetValue(m_function()); }
		};

		template <typename Func>
		class WorkInternal<void, Func, false> : public QueuedWork
		{
		private:
			SharedPtr<Future<void>> m_future;
			Func m_function;

		public:
			template <typename R>
			WorkInternal(SharedPtr<Future<void>> const& future, R&& func) : m_future(future), m_function(std::forward<R>(func)) {}
			virtual void DoWork() override { m_function(); m_future->SetValue(); }
		};

		template <typename Ret, typename Func>
		class WorkInternal<Ret, Func, true> : public QueuedWork
		{
		private:
			SharedPtr<Future<Ret>> m_future;

		public:
			template <typename R>
			WorkInternal(SharedPtr<Future<Ret>> const& future, R&& func) : m_future(future) {}
			virtual void DoWork() override { m_future->SetValue(Func()()); }
		};

		template <typename Func>
		class WorkInternal<void, Func, true> : public QueuedWork
		{
		private:
			SharedPtr<Future<void>> m_future;

		public:
			template <typename R>
			WorkInternal(SharedPtr<Future<void>> const& future, R&& func) : m_future(future) {}
			virtual void DoWork() override { Func()(); m_future->SetValue(); }
		};

		/*template <typename Ret, typename Func, bool Trivial>
		class AsyncQueuedWork : public QueuedWork
		{
		private:
			SharedPtr<WorkInternal<Ret, Func, Trivial>> m_work;

		public:
			AsyncQueuedWork(SharedPtr<WorkInternal<Ret, Func, Trivial>> const& work) : m_work(work) {}
			virtual void DoWork() override { m_work->DoWork(); }
		};*/

		template <typename Fn>
		class LambdaQueuedWork : public QueuedWork
		{
		private:
			Fn m_object;

		public:
			LambdaQueuedWork(Fn const& obj) : m_object(obj) {}
			LambdaQueuedWork(Fn&& obj) : m_object(std::move(obj)) {}
			virtual void DoWork() override { m_object(); }
		};

	public:
		static void Startup(uint32_t numThreads);
		static void Shutdown();
		static uint32_t FreeThreadCount() { return s_threadPool->FreeThreadCount(); }
		static void SubmitWork(QueuedWork* work) { s_threadPool->SubmitWork(work); }
		template <typename Fn> static void SubmitWork(Fn&& fn) { s_threadPool->SubmitWork(Mem::New<LambdaQueuedWork<std::remove_cv_t<std::remove_reference_t<Fn>>>>(std::forward<Fn>(fn))); }

		template <typename Fn>
		static auto Run(Fn&& fn) -> Task<decltype(fn())>
		{
			using Ret = decltype(fn());
			using Func = std::remove_reference_t<Fn>;
			constexpr bool TRIVIAL = concepts::TrivialEmpty<void()>;
			using WorkType = WorkInternal<Ret, Func, TRIVIAL>;

			SharedPtr<Future<Ret>> future = MakeShared<Future<Ret>>();
			WorkType* work = Mem::New<WorkType>(future, std::forward<Fn>(fn));
			s_threadPool->SubmitWork(work);
			return Task<Ret>(future);
		}
	};
}