#pragma once
#include "Core/Memory/mem.h"
#include "Core/Thread/atomic.h"
#include "Core/Thread/lock.h"
#if GLEX_REPORT_PERFORMANCE_ISSUE
#include <stdio.h>
#endif

namespace glex
{
	template <typename Elem>
	class RingBuffer
	{
	private:
		using BlockType = std::pair<Elem, bool>;

		// Be careful of cache!
		Mutex m_resizeLock;
		BlockType* m_data;
		uint32_t m_capacity;
		uint32_t m_head;
		uint32_t m_tail;

	public:
		RingBuffer(uint32_t capacity) : m_resizeLock(0)
		{
			GLEX_LASSERT(capacity > 1 && (capacity & (capacity - 1)) == 0) {}
			m_data = Mem::Alloc<BlockType>(capacity);
			memset(m_data, 0, sizeof(BlockType) * capacity);
			m_capacity = capacity;
			m_head = 0;
			m_tail = 0;
		}

		~RingBuffer()
		{
			Mem::Free(m_data);
		}

		void Push(Elem elem)
		{
		retry:
			uint32_t capacity = Atomic::Load(&m_capacity);
			uint32_t head = Atomic::Load(&m_head);
			uint32_t nextTail = Atomic::Increment(&m_tail);
			uint32_t tail = nextTail - 1 & capacity - 1;
			// Tail can overflow head at most one round.
			if (((nextTail | 0x1'0000'0000ULL) - head & 0xffffffff) >= capacity)
			{
				m_resizeLock.Lock();
				if (Atomic::Load(&m_capacity) != capacity)
				{
					m_resizeLock.Unlock();
					goto retry;		// Someone else has already expanded it. Retry.
				}
				head = Atomic::Load(&m_head);
				BlockType* newData = Mem::Alloc<BlockType>(capacity * 2);
				memset(newData, 0, sizeof(BlockType) * capacity * 2);
				head = head & capacity - 1;
				if (head < tail)
				{
					for (uint32_t i = head; i < tail; i++)
						newData[i] = m_data[i];
				}
				else
				{
					for (uint32_t i = head; i < capacity; i++)
						newData[i] = m_data[i];
					for (uint32_t i = 0; i < tail; i++)
						newData[i + capacity] = m_data[i];
					tail += capacity;
				}
				Mem::Free(m_data);
				m_data = newData;
				m_head = head;
				m_tail = tail + 1;
				m_capacity = capacity * 2;
				m_resizeLock.Unlock();
#if GLEX_REPORT_PERFORMANCE_ISSUE
				printf(GLEX_LOG_YELLOW("Render command buffer resized to %d.\n"), m_capacity);
#endif
			}
			m_data[tail].first = elem;
			Atomic::WriteBarrier();
			m_data[tail].second = true;
		}

		bool Pop(Elem& out)
		{
			ScopedLock lock(m_resizeLock);
			uint32_t capacity = Atomic::Load(&m_capacity);
			uint32_t head = Atomic::Load(&m_head);
			uint32_t tail = Atomic::Load(&m_tail);
			if ((head & capacity - 1) == (tail & capacity - 1))
				return false;
			BlockType& block = Atomic::Load(&m_data)[head & capacity - 1];
			if (!block.second)
				return false;
			block.second = false;
			out = block.first;
			Atomic::WriteBarrier();
			m_head = head + 1;
			return true;
		}
	};
}