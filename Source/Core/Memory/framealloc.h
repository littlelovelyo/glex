#pragma once
#include "Core/commdefs.h"
#include <stdint.h>

namespace glex
{

	class FrameAllocator : private Unmoveable
	{
	private:
		void* m_start;
		void* m_end;
		void* m_startCommited;
		void* m_endCommited;
		void* m_startStackPointer;
		void* m_endStackPointer;
		bool m_growFromStart;

	public:
		FrameAllocator(uint32_t size);
		~FrameAllocator();
		void* Allocate(uint32_t size, uint32_t alignment);
		void Swap();
	};

}