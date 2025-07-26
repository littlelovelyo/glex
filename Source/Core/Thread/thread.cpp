#include "thread.h"
#include <Windows.h>
#undef Yield

using namespace glex;

Thread Thread::s_mainThread;

static DWORD __stdcall ThreadProc(LPVOID param)
{
	Thread* thread = static_cast<Thread*>(param);
	thread->GetThreadProc()();
	return 0;
}

uint32_t Thread::GetThreadID()
{
	return GetCurrentThreadId();
}

void Thread::Sleep(uint32_t ms)
{
	::Sleep(ms);
}

void Thread::Yield()
{
	SwitchToThread();
}

void Thread::FillMainThread()
{
	s_mainThread.m_id = GetThreadID();
}

void Thread::CreateInternal(ThreadPriority priority)
{
	m_handle = reinterpret_cast<uint64_t>(CreateThread(nullptr, 0, ThreadProc, this, CREATE_SUSPENDED, reinterpret_cast<LPDWORD>(&m_id)));
	if (m_handle == 0)
		return;
	SetThreadPriority(reinterpret_cast<HANDLE>(m_handle), static_cast<int32_t>(priority));
}

Thread::~Thread()
{
	CloseHandle(reinterpret_cast<HANDLE>(m_handle));
}

void Thread::Suspend()
{
	SuspendThread(reinterpret_cast<HANDLE>(m_handle));
}

void Thread::Resume()
{
	ResumeThread(reinterpret_cast<HANDLE>(m_handle));
}

void Thread::Wait()
{
	WaitForSingleObject(reinterpret_cast<HANDLE>(m_handle), INFINITE);
}