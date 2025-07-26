#pragma once
#include "Core/Container/basic.h"
#include "Core/Container/optional.h"
#include <concepts>
#include <EASTL/heap.h>

namespace glex
{
	template <std::integral T>
	class UuidPool
	{
	private:
		T m_next = 0;
#if GLEX_REPORT_MEMORY_LEAKS
		union
		{
			void* m_dummy = nullptr;
			Deque<T> m_freeList;
		};
#else
		Deque<T> m_freeList;
#endif

	public:
		T Allocate()
		{
			if (!m_freeList.empty())
			{
				eastl::pop_heap(m_freeList.begin(), m_freeList.end());
				T id = m_freeList.back();
				m_freeList.pop_back();
				return id;
			}
			else
				return m_next++;
		}

		void Free(T id)
		{
			if (id == m_next - 1)
			{
				m_next--;
				while (!m_freeList.empty() && m_freeList.front() == m_next)
				{
					eastl::pop_heap(m_freeList.begin(), m_freeList.end());
					m_freeList.pop_back();
					m_next--;
				}
			}
			else
			{
				m_freeList.push_back(id);
				eastl::push_heap(m_freeList.begin(), m_freeList.end());
			}
		}

#if GLEX_REPORT_MEMORY_LEAKS
		~UuidPool()
		{
			// Do nothing.
		}

		void Construct()
		{
			new (&m_freeList) Deque<T>();
		}

		void Destruct()
		{
			m_freeList.~deque();
		}
#endif
	};
}