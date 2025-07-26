#pragma once
#include "Core/commdefs.h"
#include "Core/Memory/mem.h"
#include "Core/Container/function.h"
#include <stdint.h>

namespace glex
{
	enum class ThreadPriority : int32_t
	{
		Low = -1,
		Normal = 0,
		High = 1
	};

	class Thread : Uncopyable
	{
	private:
		static Thread s_mainThread;
		Thread() = default;

	public:
		static void FillMainThread();
		static Thread const& MainThread() { return s_mainThread; }
		static uint32_t GetThreadID();
		static void Sleep(uint32_t ms);
		static void Yield();

	private:
		uint64_t m_handle;
		uint32_t m_id;
		Function<void()> m_threadProc;

		void CreateInternal(ThreadPriority priority);

	public:
		template <typename Fn>
		Thread(Fn&& threadProc, ThreadPriority priority) : m_threadProc(std::forward<Fn>(threadProc))
		{
			CreateInternal(priority);
		}

		~Thread();
		Thread(Thread&& rhs) : m_handle(rhs.m_handle), m_id(rhs.m_id), m_threadProc(std::move(rhs.m_threadProc)) { rhs.m_handle = 0; }
		Thread& operator=(Thread&& rhs) { m_handle = rhs.m_handle; m_id = rhs.m_id; m_threadProc = std::move(rhs.m_threadProc); rhs.m_handle = 0; return *this; }
		bool IsValid() const { return m_handle != 0; }
		uint32_t ID() const { return m_id; }
		Function<void()> const& GetThreadProc() const { return m_threadProc; }
		void Suspend();
		void Resume();
		void Wait();
	};
}