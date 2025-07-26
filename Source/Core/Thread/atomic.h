#pragma once
#include "Core/commdefs.h"
#include <intrin.h>
#include <concepts>
#include <atomic>

namespace glex
{
	class Atomic : private StaticClass
	{
	public:
		// New value is returned.
		template <concepts::SizeIs<4> T> requires std::is_integral_v<T>
		static T Increment(T* p)
		{
			return _InterlockedIncrement(reinterpret_cast<long*>(p));
		}

		// New value is returned.
		template <concepts::SizeIs<4> T> requires std::is_integral_v<T>
		static T Decrement(T* p)
		{
			return _InterlockedDecrement(reinterpret_cast<long*>(p));
		}

		template <concepts::SizeIs<4> T> requires std::is_integral_v<T>
		static T Add(T* p, T v)
		{
			return _InterlockedExchangeAdd(reinterpret_cast<long*>(p), v);
		}

		template <concepts::SizeIs<8> T> requires std::is_integral_v<T>
		static T And(T* p, T v)
		{
			return _InterlockedAnd64(reinterpret_cast<long long*>(p), v);
		}

		template <typename T, std::convertible_to<T*> K>
		static T* Exchange(T** p, K v)
		{
			return static_cast<T*>(_InterlockedExchangePointer(p, v));
		}

		template <concepts::SizeIs<1> T>
		static T Exchange(T* p, T v)
		{
			return _InterlockedExchange8(reinterpret_cast<char*>(p), v);
		}

		template <concepts::SizeIs<4> T>
		static T Exchange(T* p, T v)
		{
			return _InterlockedExchange(reinterpret_cast<long*>(p), v);
		}

		template <concepts::SizeIs<1> T>
		static T CompareAndExchange(T* p, T cmp, T chg)
		{
			return _InterlockedCompareExchange8(reinterpret_cast<char*>(p), chg, cmp);
		}

		template <concepts::SizeIs<8> T>
		static T CompareAndExchange(T* p, T cmp, T chg) requires (!std::is_pointer_v<T>)
		{
			return _InterlockedCompareExchange64(reinterpret_cast<long long*>(p), chg, cmp);
		}

		template <typename T, std::convertible_to<T*> K>
		static T* CompareAndExchange(T** p, K cmp, K chg)
		{
			return reinterpret_cast<T*>(_InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(p), chg, cmp));
		}

		template <typename T>
		static T Load(T const* p)
		{
			return *static_cast<T const volatile*>(p);
		}

		static void ReadBarrier()
		{
			std::atomic_thread_fence(std::memory_order_acquire);
		}

		static void WriteBarrier()
		{
			std::atomic_thread_fence(std::memory_order_release);
		}

		static void ReadWriteBarrier()
		{
			std::atomic_thread_fence(std::memory_order_acq_rel);
		}
	};
}