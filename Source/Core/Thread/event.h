#pragma once
#include "Core/commdefs.h"

namespace glex
{

	class Event : Unmoveable
	{
	private:
		uint64_t m_handle;
		bool m_manualReset;

	public:
		// Used for event pooling.
		static Event* Get(bool manualReset);
		static void Release(Event* event);

#if GLEX_INTERNAL
		static void Clear();
#endif
		Event(bool manualReset);
		~Event();
		Event(Event&& rhs) : m_handle(rhs.m_handle), m_manualReset(rhs.m_manualReset) { rhs.m_handle = 0; };
		Event& operator=(Event&& rhs) { m_handle = rhs.m_handle; rhs.m_handle = 0; return *this; }
		bool operator==(nullptr_t rhs) const { return m_handle == 0; }
		uint64_t Handle() const { return m_handle; }
		bool IsManualReset() const { return m_manualReset; }
		void Wait();
		void Set();
		void Reset();
	};

}