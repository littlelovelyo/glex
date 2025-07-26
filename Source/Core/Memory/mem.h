/**
 * Generic definitions and platform-specific memory management tools.
 * But here we only supports Win32 to simplify things.
 */
#pragma once
#include "Core/commdefs.h"
#include "config.h"
#if GLEX_REPORT_MEMORY_LEAKS
#include <vector>
#include <optional>
#include <stacktrace>
#include <unordered_map>
#endif

namespace glex
{
	class Mem : private StaticClass
	{
#if GLEX_REPORT_MEMORY_LEAKS
	private:
		inline static std::optional<std::vector<FunctionPtr<void()>>> s_freeMemoryCallbacks;
		inline static std::unordered_map<void*, std::stacktrace> s_traces, s_newTraces;
		inline static std::unordered_map<void*, std::vector<std::stacktrace>> s_doubleFrees;
		inline static uint64_t s_addrXor;
		inline static uint64_t s_addrSum;
		inline static int64_t s_allocTimes;
		inline static int64_t s_newTimes;
		static bool s_traceMemoryAllocation;

		static void TraceAlloc(void* p);
		static void TraceFree(void* p);

	public:
		static void Report();
		static void RegisterFreeMemory(FunctionPtr<void()> fn);
		static void FreeMemory();
#endif

	public:
		constexpr static uint32_t k_pageSize = 4096;

		inline constexpr static bool IsAligned(uint32_t size, uint32_t alignment)
		{
			return alignment == 0 || (size & alignment - 1) == 0;
		}

		inline constexpr static bool IsAligned(uint64_t addr, uint32_t alignment)
		{
			return alignment == 0 || (addr & alignment - 1) == 0;
		}

		inline constexpr static bool IsAligned(void const* addr, uint32_t alignment)
		{
			return IsAligned(reinterpret_cast<uint64_t>(addr), alignment);
		}

		inline constexpr static uint32_t Align(uint32_t size, uint32_t alignment)
		{
			return alignment == 0 ? size : (size + (alignment - 1)) & ~(alignment - 1);
		}

		inline constexpr static uint32_t DownAlign(uint32_t size, uint32_t alignment)
		{
			return alignment == 0 ? size : size & ~(alignment - 1);
		}

		inline constexpr static uint64_t Align(uint64_t addr, uint32_t alignment)
		{
			uint64_t align = alignment;
			return align == 0 ? addr : (addr + (align - 1)) & ~(align - 1);
		}

		inline constexpr static uint64_t DownAlign(uint64_t addr, uint32_t alignment)
		{
			uint64_t align = alignment;
			return align == 0 ? addr : addr & ~(align - 1);
		}

		inline static void* Align(void const* addr, uint32_t alignemnt)
		{
			return reinterpret_cast<void*>(Align(reinterpret_cast<uint64_t>(addr), alignemnt));
		}

		inline static void* DownAlign(void const* addr, uint32_t alignment)
		{
			return reinterpret_cast<void*>(DownAlign(reinterpret_cast<uint64_t>(addr), alignment));
		}

		template <typename T = void>
		inline static T* Offset(void const* addr, uint32_t size)
		{
			return reinterpret_cast<T*>(reinterpret_cast<uint64_t>(addr) + size);
		}

		template <typename T = void>
		inline static T* DownOffset(void const* addr, uint32_t size)
		{
			return reinterpret_cast<T*>(reinterpret_cast<uint64_t>(addr) - size);
		}

		template <typename T>
		inline static uint32_t Diff(T const* start, T const* end)
		{
			return reinterpret_cast<uint64_t>(end) - reinterpret_cast<uint64_t>(start);
		}

		static void* Alloc(uint64_t size);
		static void* Alloc(uint64_t size, uint32_t alignment);
		static void* Alloc(uint64_t size, uint32_t alignment, uint32_t offset);
		static void* Realloc(void* p, uint64_t size);
		static void Free(void* addr);
		static void* AllocPages(uint64_t size);
		static void CommitPages(void* addr, uint64_t size);
		static void DecommitPages(void* addr, uint64_t size);
		static void FreePages(void* addr);
		static void OutOfMemory();

		template <typename T>
		static T* Alloc()
		{
			return static_cast<T*>(Alloc(sizeof(T), alignof(T)));
		}

		template <typename T>
		static T* Alloc(uint64_t count)
		{
			return static_cast<T*>(Alloc(sizeof(T) * count, alignof(T)));
		}

		template <typename T, typename... Args>
		static T* New(Args&&... args)
		{
#if GLEX_REPORT_MEMORY_LEAKS
			s_newTimes++;
#endif
			T* p = Alloc<T>();
			new(p) T(std::forward<Args>(args)...);
#if GLEX_REPORT_MEMORY_LEAKS
			if (s_traceMemoryAllocation)
				s_newTraces[p] = std::stacktrace::current();
#endif
			return p;
		}

		template <typename T, typename... Args>
		static T* SizedNew(uint64_t sizeOverride, Args&&... args)
		{
#if GLEX_REPORT_MEMORY_LEAKS
			s_newTimes++;
#endif
			T* p = static_cast<T*>(Alloc(sizeOverride, alignof(T)));
			new(p) T(std::forward<Args>(args)...);
#if GLEX_REPORT_MEMORY_LEAKS
			if (s_traceMemoryAllocation)
				s_newTraces[p] = std::stacktrace::current();
#endif
			return p;
		}

		template <typename T>
		static void Delete(T* p)
		{
#if GLEX_REPORT_MEMORY_LEAKS
			if (p != nullptr)
			{
				s_newTimes--;
				auto iter = s_newTraces.find(p);
				if (iter != s_newTraces.end())
					s_newTraces.erase(iter);
				else
					s_doubleFrees[p].push_back(std::stacktrace::current());
			}
#endif
			p->~T();
			Mem::Free(p);
		}
	};
}