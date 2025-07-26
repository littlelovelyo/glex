#pragma once
#include "Core/Memory/mem.h"

namespace glex
{
	class Allocator
	{
	private:
#if GLEX_LDEBUG
		char const* m_name;
#endif

	public:
#if GLEX_LDEBUG
		Allocator(char const* name = nullptr) : m_name(name) {}
		char const* get_name() const { return m_name; }
		void set_name(char const* name) { m_name = name; }
#else
		Allocator(char const* name = nullptr) {}
		char const* get_name() const { return nullptr; }
		void set_name(char const* name) {}
#endif

		void* allocate(uint32_t n, int flags = 0)
		{
			return Mem::Alloc(n);
		}

		void* allocate(uint32_t n, uint32_t alignment, uint32_t offset, int flags = 0)
		{
			return Mem::Alloc(n, alignment, offset);
		}

		void  deallocate(void* p, uint32_t n)
		{
			Mem::Free(p);
		}

		bool operator==(Allocator const& rhs) const
		{
			return true;
		}
	};

	// Shitty std allocator.
	template <typename T>
	struct StdAllocator
	{
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using propagate_on_container_move_assignment = std::true_type;
		using is_always_equal = std::true_type;

		StdAllocator() = default;
		template <typename R> StdAllocator(StdAllocator<R> const& rhs) {}

		T* allocate(uint32_t size)
		{
			return Mem::Alloc<T>(size);
		}

		void deallocate(T* p, uint32_t n)
		{
			Mem::Free(p);
		}

		template <typename R>
		constexpr bool operator==(StdAllocator<R> const& rhs) const
		{
			return true;
		}
	};
}