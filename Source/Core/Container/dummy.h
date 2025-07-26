#pragma once
#include <stdint.h>

namespace glex
{
	template <uint32_t size, uint32_t alignment>
	class alignas(alignment) Dummy
	{
	private:
		uint8_t m_data[size];

	public:
		template <typename T>
		T& As()
		{
			return *reinterpret_cast<T*>(m_data);
		}
	};

	template <typename T>
	class alignas(alignof(T)) AbstructHolder
	{
	private:
		uint8_t m_data[sizeof(T)];

	public:
		T& Get()
		{
			return *reinterpret_cast<T*>(m_data);
		}

		T* operator&()
		{
			return reinterpret_cast<T*>(m_data);
		}
	};
}