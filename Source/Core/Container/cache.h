/**
 * A general-purpose LRU cache with lifetime for each element.
 * Try to make it thread-safe.
 */
#pragma once
#include "config.h"
#include "Core/Memory/allocator.h"
#include "Core/Thread/lock.h"
#include "Core/Container/optional.h"
#include "Core/Memory/smart_ptr.h"
#include "Core/Container/basic.h"

namespace glex
{
	template <typename Elem>
	class Cache : private Unmoveable
	{
	private:
		struct Slot
		{
			String name;
			uint32_t hash;	// hash cache, we use linear search because size isn't too big.
			uint32_t life;	// 0 if slot is free.
			SharedPtr<Elem> elem;
		};

		Mutex m_mutex;
		Vector<Slot> m_slots;
		uint32_t m_init;
		uint16_t m_boost, m_decrease;

	public:
		Cache(uint32_t size, uint32_t spinCount, uint32_t init, uint16_t boost, uint16_t decrease) :
			m_mutex(spinCount), m_slots(size), m_init(init), m_boost(boost), m_decrease(decrease) {}

		template <typename... Args>
		SharedPtr<Elem> Get(char const* name, Args&&... args)
		{
			eastl::hash<char const*> hasher;
			uint32_t hash = hasher(name);
			uint32_t minLife = -1;
			uint32_t minIndex = -1;
			ScopedLock lock(m_mutex);
			for (uint32_t i = 0; i < m_slots.size(); i++)
			{
				Slot& slot = m_slots[i];
				if (slot.life != 0 && slot.hash == hash && slot.name == name)
				{
					slot.life = std::min(m_init, slot.life + m_boost);
					return slot.elem;
				}
				if (slot.life < minLife)
				{
					minLife = slot.life;
					minIndex = i;
				}
			}
			Slot& slot = m_slots[minIndex];
			slot.name = name;
			slot.hash = hash;
			slot.life = m_init;
			slot.elem = MakeShared<Elem>(std::forward<Args>(args)...);
			return slot.elem;
		}

		void ReduceLifetime()
		{
			ScopedLock lock(m_mutex);
			for (Slot& slot : m_slots)
			{
				if (slot.life > m_decrease)
					slot.life -= m_decrease;
				else
				{
					slot.life = 0;
					slot.elem = nullptr;
				}
			}
		}
	};
}