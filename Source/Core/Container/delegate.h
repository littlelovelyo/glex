/**
 * Delegates are just thread-safe functions.
 */
#pragma once
#include "config.h"
#include "Core/Container/basic.h"
#include "Core/Container/function.h"
#include "Core/Thread/lock.h"

namespace glex
{
	template <typename Fn>
	class Delegate;

	template <typename Ret, typename... Args>
	class Delegate<Ret(Args...)>
	{
	private:
		mutable Mutex m_mutex = 1024;
		Vector<Function<Ret(Args...)>> m_callbacks;

	public:
		template <typename Fn>
		void Bind(Fn&& fn)
		{
			ScopedLock lock(m_mutex);
			m_callbacks.emplace_back(std::forward<Fn>(fn));
		}

		template <typename Obj>
		void Bind(Obj* obj, MemberFunctionPtr<Obj, Ret(Args...)> memFn)
		{
			ScopedLock lock(m_mutex);
			m_callbacks.emplace_back(obj, memFn);
		}

		template <typename Fn>
		void Unbind(Fn const& fn)
		{
			ScopedLock lock(m_mutex);
			for (auto iter = m_callbacks.begin(); iter != m_callbacks.end(); ++iter)
			{
				if (*iter == fn)
				{
					m_callbacks.erase_unsorted(iter);
					break;
				}
			}
		}

		template <typename Obj>
		void Unbind(Obj const* obj, MemberFunctionPtr<Obj, Ret(Args...)> memFn)
		{
			ScopedLock lock(m_mutex);
			for (auto iter = m_callbacks.begin(); iter != m_callbacks.end(); ++iter)
			{
				if (iter->Equals(obj, memFn))
				{
					m_callbacks.erase_unsorted(iter);
					break;
				}
			}
		}

		void Broadcast(Args... args) const
		{
			// Fast path.
			if (m_callbacks.empty())
				return;
			ScopedLock lock(m_mutex);
			for (auto& fn : m_callbacks)
				fn(std::forward<Args>(args)...);
		}

#if GLEX_REPORT_MEMORY_LEAKS
		void Clear()
		{
			m_callbacks.clear();
			m_callbacks.shrink_to_fit();
		}
#endif
	};
}