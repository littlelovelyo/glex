#pragma once
#include "Core/commdefs.h"
#include <utility>

namespace glex
{
	class StackAllocator : private Unmoveable
	{
	private:
		void* m_initPagesPtr;
		void* m_endPagesPtr;
		void* m_endCommitedPtr;
		void* m_stackPtr;

	public:
		class Bookmark
		{
		private:
			StackAllocator& m_owner;
			void* m_previous;

		public:
			Bookmark(StackAllocator& owner, void* previous) : m_owner(owner), m_previous(previous) {}
			~Bookmark() { m_owner.m_stackPtr = m_previous; }
		};

		StackAllocator(uint32_t size);
		~StackAllocator();
		std::pair<void*, Bookmark> Allocate(uint32_t size, uint32_t alignment);
	};
}