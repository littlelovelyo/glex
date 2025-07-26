#include "Core/Memory/stackalloc.h"
#include "Core/Memory/mem.h"

using namespace glex;

StackAllocator::StackAllocator(uint32_t size)
{
	m_initPagesPtr = Mem::AllocPages(Mem::Align(size, Mem::k_pageSize));
	m_endPagesPtr = Mem::Offset(m_initPagesPtr, size);
	m_endCommitedPtr = m_initPagesPtr;
	m_stackPtr = m_initPagesPtr;
}

StackAllocator::~StackAllocator()
{
	Mem::FreePages(m_initPagesPtr);
}

std::pair<void*, StackAllocator::Bookmark> StackAllocator::Allocate(uint32_t size, uint32_t alignemnt)
{
	void* previous = m_stackPtr;
	void* start = Mem::Align(m_stackPtr, alignemnt);
	void* end = Mem::Offset(start, size);
	if (end > m_endPagesPtr)
		Mem::OutOfMemory();
	if (m_endCommitedPtr < end)
	{
		uint32_t commitSize = Mem::Align(Mem::Diff(m_endCommitedPtr, end), Mem::k_pageSize);
		Mem::CommitPages(m_endCommitedPtr, commitSize);
		m_endCommitedPtr = Mem::Offset(m_endCommitedPtr, commitSize);
	}
	m_stackPtr = end;
	return { start, Bookmark(*this, previous) };
}