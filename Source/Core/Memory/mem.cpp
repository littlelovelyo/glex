#include "Core/Memory/mem.h"
#include "Core/assert.h"
#include "Core/Thread/thread.h"
#include <mimalloc.h>
#include <Windows.h>
#if GLEX_REPORT_MEMORY_LEAKS
#include <string>
#endif

using namespace glex;

#if GLEX_REPORT_MEMORY_LEAKS
void Mem::TraceAlloc(void* p)
{
	s_addrXor ^= reinterpret_cast<uint64_t>(p);
	s_addrSum += reinterpret_cast<uint64_t>(p);
	s_allocTimes++;
	if (s_traceMemoryAllocation)
		s_traces[p] = std::stacktrace::current();
}

void Mem::TraceFree(void* p)
{
	if (p == nullptr)
		return;
	s_addrXor ^= reinterpret_cast<uint64_t>(p);
	s_addrSum -= reinterpret_cast<uint64_t>(p);
	s_allocTimes--;
	if (s_allocTimes < 0)
		Logger::Error("Some pointers are freed more than once.");
	if (s_traceMemoryAllocation)
	{
		auto iter = s_traces.find(p);
		if (iter != s_traces.end())
			s_traces.erase(iter);
		else
			s_doubleFrees[p].push_back(std::stacktrace::current());
	}
}
#endif

void* Mem::Alloc(uint64_t size)
{
	void* p = mi_malloc(size);
	if (p == nullptr)
		OutOfMemory();
#if GLEX_REPORT_MEMORY_LEAKS
	TraceAlloc(p);
#endif
	return p;
}

void* Mem::Alloc(uint64_t size, uint32_t alignment)
{
	void* p = mi_malloc_aligned(size, alignment);
	if (p == nullptr)
		OutOfMemory();
#if GLEX_REPORT_MEMORY_LEAKS
	TraceAlloc(p);
#endif
	return p;
}

void* Mem::Alloc(uint64_t size, uint32_t alignment, uint32_t offset)
{
	void* p = mi_malloc_aligned_at(size, alignment, offset);
	if (p == nullptr)
		OutOfMemory();
#if GLEX_REPORT_MEMORY_LEAKS
	TraceAlloc(p);
#endif
	return p;
}

void* Mem::Realloc(void* p, uint64_t size)
{
	void* np = mi_realloc(p, size);
	if (np == nullptr)
		OutOfMemory();
#if GLEX_REPORT_MEMORY_LEAKS
	TraceFree(p);
	TraceAlloc(np);
#endif
	return np;
}

void Mem::Free(void* addr)
{
	mi_free(addr);
#if GLEX_REPORT_MEMORY_LEAKS
	TraceFree(addr);
#endif
}

#if GLEX_REPORT_MEMORY_LEAKS
void Mem::Report()
{
	if (s_freeMemoryCallbacks.has_value())
	{
		for (auto fn : *s_freeMemoryCallbacks)
			fn();
		s_freeMemoryCallbacks.reset();
	}
	if (s_addrXor != 0 || s_addrSum != 0 || s_allocTimes != 0)
		Logger::Error("There're memory leaks. %ld pointers are not freed.", s_allocTimes);
	else if (s_newTimes != 0)
		Logger::Error("%d New/Delete mismatches.", s_newTimes);
	else if (!s_traceMemoryAllocation || s_traces.empty() && s_doubleFrees.empty())
		Logger::Debug("No memory leaks detected. ( ^)o(^ )");

	// This is useless.	
	if (s_traceMemoryAllocation)
	{
		if (!s_traces.empty())
		{
			Logger::Debug("\n    LEAKED POINTERS");
			for (auto& [key, st] : s_traces)
			{
				Logger::Debug(R"(+-----------------------------+
| Pointer: 0x%p |
+-----------------------------+)", key);
				Logger::Debug(std::to_string(st).c_str());
				Logger::Debug("--------------------------------------------------------------");
			}
			for (auto& [key, st] : s_newTraces)
			{
				Logger::Debug(R"(+-----------------------------+
| Pointer: 0x%p |
+-----------------------------+)", key);
				Logger::Debug(std::to_string(st).c_str());
				Logger::Debug("--------------------------------------------------------------");
			}
		}
		if (!s_doubleFrees.empty())
		{
			Logger::Debug("\n    DOUBLE-FREED POINTERS");
			for (auto& [key, list] : s_doubleFrees)
			{
				Logger::Debug(R"(+-----------------------------+
| Pointer: 0x%p |
+-----------------------------+)", key);
				for (auto& st : list)
				{
					Logger::Debug(std::to_string(st).c_str());
					Logger::Debug("--------------------------------------------------------------");
				}
			}
		}
	}
}

void Mem::RegisterFreeMemory(FunctionPtr<void()> fn)
{
	if (!s_freeMemoryCallbacks.has_value())
		s_freeMemoryCallbacks.emplace();
	s_freeMemoryCallbacks->push_back(fn);
}
#endif

void* Mem::AllocPages(uint64_t size)
{
	GLEX_DEBUG_ASSERT(size != 0 && IsAligned(size, k_pageSize)) {}
	void* p = VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
	if (p == nullptr)
		OutOfMemory();
	return p;
}

void Mem::CommitPages(void* addr, uint64_t size)
{
	GLEX_DEBUG_ASSERT(IsAligned(addr, k_pageSize)) {}
	GLEX_DEBUG_ASSERT(size != 0 && IsAligned(size, k_pageSize)) {}
	if (VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE) == nullptr)
		OutOfMemory();
}

void Mem::DecommitPages(void* addr, uint64_t size)
{
	GLEX_DEBUG_ASSERT(IsAligned(addr, k_pageSize)) {}
	GLEX_DEBUG_ASSERT(size != 0 && IsAligned(size, k_pageSize)) {}
	BOOL ret = VirtualFree(addr, size, MEM_DECOMMIT);
	GLEX_DEBUG_ASSERT(ret);
}

void Mem::FreePages(void* addr)
{
	BOOL ret = VirtualFree(addr, 0, MEM_RELEASE);
	GLEX_DEBUG_ASSERT(ret);
}

void Mem::OutOfMemory()
{
	Logger::Fatal("Out of memory!");
}