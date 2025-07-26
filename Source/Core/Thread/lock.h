#pragma once
#include "Core/commdefs.h"
#include "Core/Container/dummy.h"

namespace glex
{
	template <typename T>
	class ScopedLock : Unmoveable
	{
	private:
		T* m_lock;

	public:
		ScopedLock(T& lock) : m_lock(&lock) { m_lock->Lock(); }
		~ScopedLock() { m_lock->Unlock(); }
	};

	class Mutex : private Unmoveable
	{
	private:
		// We use this to avoid including Windows.h.
		Dummy<40, 8> m_criticalSection;

	public:
		Mutex(uint32_t spinCount);
		~Mutex();
		void Lock();
		void Unlock();
	};
}