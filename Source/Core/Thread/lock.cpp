#include "lock.h"
#include <Windows.h>

using namespace glex;

Mutex::Mutex(uint32_t spinCount)
{
	InitializeCriticalSectionAndSpinCount(&m_criticalSection.As<CRITICAL_SECTION>(), spinCount);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&m_criticalSection.As<CRITICAL_SECTION>());
}

void Mutex::Lock()
{
	EnterCriticalSection(&m_criticalSection.As<CRITICAL_SECTION>());
}

void Mutex::Unlock()
{
	LeaveCriticalSection(&m_criticalSection.As<CRITICAL_SECTION>());
}