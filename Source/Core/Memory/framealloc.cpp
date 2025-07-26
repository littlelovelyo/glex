#include "Core/Memory/framealloc.h"
#include "Core/Memory/mem.h"
#include "Core/log.h"

using namespace glex;

FrameAllocator::FrameAllocator(uint32_t size)
{
	m_start = Mem::AllocPages(size);
	m_end = Mem::Offset(m_start, size);
	m_startCommited = m_start;
	m_endCommited = m_end;
	m_startStackPointer = m_start;
	m_endStackPointer = m_end;
	m_growFromStart = true;
}

FrameAllocator::~FrameAllocator()
{
	Mem::FreePages(m_start);
}

void* FrameAllocator::Allocate(uint32_t size, uint32_t alignment)
{
	if (m_growFromStart)
	{
		void* ptr = Mem::Align(m_startStackPointer, alignment);
		m_startStackPointer = Mem::Offset(ptr, size);
		if (m_startStackPointer > m_endStackPointer)
			Logger::Fatal("Frame allocator is too small. Current size: %d. Allocating: %d.", Mem::Diff(m_start, m_end), size);
		if (m_startStackPointer > m_startCommited)
		{
			void* end = Mem::Align(m_startStackPointer, Mem::k_pageSize);
			if (end > m_endCommited) [[unlikely]]
			{
				end = m_endCommited;
				uint32_t commitSize = Mem::Diff(m_startCommited, end);
				Mem::CommitPages(m_startCommited, commitSize);
				m_startCommited = m_end;
				m_endCommited = m_start;
			}
			else
			{
				uint32_t commitSize = Mem::Diff(m_startCommited, end);
				Mem::CommitPages(m_startCommited, commitSize);
				m_startCommited = end;
			}
		}
		return ptr;
	}
	else
	{
		m_endStackPointer = Mem::DownAlign(Mem::DownOffset(m_endStackPointer, size), alignment);
		if (m_endStackPointer < m_startStackPointer)
			Logger::Fatal("Frame allocator is too small. Current size: %d. Allocating: %d.", Mem::Diff(m_start, m_end), size);
		if (m_endStackPointer < m_endCommited)
		{
			void* start = Mem::DownAlign(m_endStackPointer, Mem::k_pageSize);
			if (start < m_startCommited) [[unlikely]]
			{
				start = m_startCommited;
				uint32_t commitSize = Mem::Diff(start, m_endCommited);
				Mem::CommitPages(start, commitSize);
				m_startCommited = m_end;
				m_endCommited = m_start;
			}
			else
			{
				uint32_t commitSize = Mem::Diff(start, m_endCommited);
				Mem::CommitPages(start, commitSize);
				m_endCommited = start;
			}
		}
		return m_endStackPointer;
	}
}

void FrameAllocator::Swap()
{
	if (m_growFromStart)
	{
		m_endStackPointer = m_end;
		m_growFromStart = false;
	}
	else
	{
		m_startStackPointer = m_start;
		m_growFromStart = true;
	}
}