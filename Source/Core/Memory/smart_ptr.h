/**
 * Implements thread-safe shared pointer.
 * Shared pointer is implemented differently from std::shared_ptr
 * to use custom memory allocation.
 * Control block is placed exactly before data block.
 */
#pragma once
#include "Core/Memory/mem.h"
#include "Core/Thread/atomic.h"
#include "Core/commdefs.h"
#include <EASTL/functional.h>

namespace glex
{
	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			SHARED_PTR
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename T>
	class SharedPtr
	{
		template <typename R>
		friend class SharedPtr;

		template <typename R>
		friend class WeakPtr;

		template <typename R, typename... Args>
		friend SharedPtr<R> MakeShared(Args&&... args);

	private:
		uint32_t* m_controlBlock;
		T* m_pointer;

		SharedPtr(uint32_t* controlBlolck, T* pointer) : m_controlBlock(controlBlolck), m_pointer(pointer) {}

	public:
		SharedPtr() : SharedPtr(nullptr, nullptr) {}
		SharedPtr(nullptr_t rhs) : SharedPtr() {}

		~SharedPtr()
		{
			if (m_controlBlock != nullptr && Atomic::Decrement(m_controlBlock) == 0)
			{
				m_pointer->~T();
				Mem::Free(m_controlBlock);
			}
		}

		SharedPtr(SharedPtr<T> const& rhs) : m_controlBlock(rhs.m_controlBlock), m_pointer(rhs.m_pointer)
		{
			if (m_controlBlock != nullptr)
				Atomic::Increment(m_controlBlock);
		}

		template <typename R> requires std::convertible_to<R*, T*>
		SharedPtr(SharedPtr<R> const& rhs) : m_controlBlock(rhs.m_controlBlock), m_pointer(rhs.m_pointer)
		{
			if (m_controlBlock != nullptr)
				Atomic::Increment(m_controlBlock);
		}

		SharedPtr(SharedPtr<T>&& rhs) : m_controlBlock(rhs.m_controlBlock), m_pointer(rhs.m_pointer)
		{
			rhs.m_controlBlock = nullptr;
			rhs.m_pointer = nullptr;
		}

		template <typename R> requires std::convertible_to<R*, T*>
		SharedPtr(SharedPtr<R>&& rhs) : m_controlBlock(rhs.m_controlBlock), m_pointer(rhs.m_pointer)
		{
			rhs.m_controlBlock = nullptr;
			rhs.m_pointer = nullptr;
		}

		SharedPtr<T>& operator=(SharedPtr<T> const& rhs)
		{
			if (m_controlBlock != nullptr && Atomic::Decrement(m_controlBlock) == 0)
			{
				m_pointer->~T();
				Mem::Free(m_controlBlock);
			}
			m_controlBlock = rhs.m_controlBlock;
			m_pointer = rhs.m_pointer;
			if (m_controlBlock != nullptr)
				Atomic::Increment(m_controlBlock);
			return *this;
		}

		template <typename R> requires std::convertible_to<R*, T*>
		SharedPtr<T>& operator=(SharedPtr<R> const& rhs)
		{
			if (m_controlBlock != nullptr && Atomic::Decrement(m_controlBlock) == 0)
			{
				m_pointer->~T();
				Mem::Free(m_controlBlock);
			}
			m_controlBlock = rhs.m_controlBlock;
			m_pointer = rhs.m_pointer;
			if (m_controlBlock != nullptr)
				Atomic::Increment(m_controlBlock);
			return *this;
		}

		SharedPtr<T>& operator=(SharedPtr<T>&& rhs)
		{
			std::swap(m_controlBlock, rhs.m_controlBlock);
			std::swap(m_pointer, rhs.m_pointer);
			return *this;
		}

		template <typename R> requires std::convertible_to<R*, T*>
		SharedPtr<T>& operator=(SharedPtr<R>&& rhs)
		{
			std::swap(m_controlBlock, rhs.m_controlBlock);
			std::swap(m_pointer, rhs.m_pointer);
			return *this;
		}

		SharedPtr<T>& operator=(nullptr_t rhs)
		{
			if (m_controlBlock != nullptr && Atomic::Decrement(m_controlBlock) == 0)
			{
				m_pointer->~T();
				Mem::Free(m_controlBlock);
			}
			m_controlBlock = nullptr;
			m_pointer = nullptr;
			return *this;
		}

		bool operator==(nullptr_t rhs) const
		{
			return m_pointer == nullptr;
		}

		template <typename R> requires concepts::InplaceCastable<R, T>
		bool operator==(SharedPtr<R> const& rhs) const
		{
			return m_controlBlock == rhs.m_controlBlock;
		}

		template <typename R>
		bool operator<(SharedPtr<R> const& rhs) const
		{
			return m_controlBlock < rhs.m_controlBlock;
		}

		T* Get() const
		{
			return m_pointer;
		}

		T* operator->() const
		{
			return m_pointer;
		}

		T& operator*() const
		{
			return *m_pointer;
		}
	};

	template <typename T>
	constexpr uint32_t CONTROL_BLOCK_SIZE_AND_ALIGNMENT = std::max<uint32_t>(4, alignof(T));

	template <typename T, typename... Args>
	SharedPtr<T> MakeShared(Args&&... args)
	{
		constexpr uint32_t align = CONTROL_BLOCK_SIZE_AND_ALIGNMENT<T>;
		uint32_t* controlBlock = static_cast<uint32_t*>(Mem::Alloc(sizeof(T) + align, align));
		*controlBlock = 1;
		T* pointer = Mem::Offset<T>(controlBlock, align);
		new (pointer) T(std::forward<Args>(args)...);
		return SharedPtr<T>(controlBlock, pointer);
	}

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			UNIQUE_PTR
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename T>
	class UniquePtr : Uncopyable
	{
		template <typename R, typename... Args>
		friend UniquePtr<R> MakeUnique(Args&&... args);

		template <typename R>
		friend UniquePtr<R> UniqueOf(R* ptr);

		template <typename R, typename S>
		friend UniquePtr<R> ReinterpretCast(UniquePtr<S>&& ptr);

		template <typename R>
		friend class UniquePtr;

	private:
		T* m_pointer;

		UniquePtr(T* pointer) : m_pointer(pointer) {}

	public:
		UniquePtr() : m_pointer(nullptr) {}
		UniquePtr(nullptr_t rhs) : m_pointer(nullptr) {}
		~UniquePtr() { if (m_pointer != nullptr) Mem::Delete(const_cast<std::remove_const_t<T>*>(m_pointer)); }
		UniquePtr(UniquePtr<T>&& rhs) : m_pointer(rhs.m_pointer) { rhs.m_pointer = nullptr; }
		UniquePtr<T>& operator=(UniquePtr<T>&& rhs) { std::swap(m_pointer, rhs.m_pointer); return *this; }
		UniquePtr<T>& operator=(nullptr_t rhs) { if (m_pointer != nullptr) { Mem::Delete(m_pointer); m_pointer = nullptr; } return *this; }
		template <typename R> requires std::convertible_to<R*, T*> UniquePtr(UniquePtr<R>&& rhs) : m_pointer(rhs.m_pointer) { rhs.m_pointer = nullptr; }
		template <typename R> requires std::convertible_to<R*, T*> UniquePtr<T>& operator=(UniquePtr<R>&& rhs) { std::swap(m_pointer, rhs.m_pointer); return *this; }
		T* operator->() const { return m_pointer; }
		T& operator*() const { return *m_pointer; }
		T* Get() const { return m_pointer; }
		bool operator==(UniquePtr<T> const& rhs) const { return m_pointer == rhs.m_pointer; }
		bool operator<(UniquePtr<T> const& rhs) const { return m_pointer < rhs.m_pointer; }
		bool operator==(T* const& rhs) const { return m_pointer == rhs; }
		bool operator<(T* const& rhs) const { return m_pointer < rhs; }
	};

	template <typename T, typename... Args>
	UniquePtr<T> MakeUnique(Args&&... args)
	{
		T* ptr = Mem::New<T>(std::forward<Args>(args)...);
		return UniquePtr<T>(ptr);
	}

	template <typename T>
	UniquePtr<T> UniqueOf(T* ptr)
	{
		return UniquePtr<T>(ptr);
	}

	template <typename T, typename R>
	UniquePtr<T> ReinterpretCast(UniquePtr<R>&& ptr)
	{
		UniquePtr<T> ret(reinterpret_cast<T*>(ptr.m_pointer));
		ptr.m_pointer = nullptr;
		return ret;
	}

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			WEAK_PTR
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename T>
	class WeakPtr
	{
	private:
		T* m_pointer;

	public:
		WeakPtr() : m_pointer(nullptr) {}
		WeakPtr(T* pointer) : m_pointer(pointer) {}
		template <typename R> requires std::convertible_to<R*, T*> WeakPtr(SharedPtr<R> const& sharedPtr) : m_pointer(sharedPtr.Get()) {}
		template <typename R> requires std::convertible_to<R*, T*> WeakPtr(UniquePtr<R> const& uniquePtr) : m_pointer(uniquePtr.Get()) {}
		T* operator->() const { return m_pointer; }
		T& operator*() const { return *m_pointer; }
		T* Get() const { return m_pointer; }
		bool operator==(WeakPtr<T> const& rhs) const { return m_pointer == rhs.m_pointer; }
		bool operator<(WeakPtr<T> const& rhs) const { return m_pointer < rhs.m_pointer; }
		bool operator==(UniquePtr<T> const& rhs) const { return m_pointer == rhs.Get(); }
		bool operator<(UniquePtr<T> const& rhs) const { return m_pointer < rhs.Get(); }
		bool operator==(T* const& rhs) const { return m_pointer == rhs; }
		bool operator<(T* const& rhs) const { return m_pointer < rhs; }
		operator T*() const { return m_pointer; }
		template <typename R> operator WeakPtr<R>() const requires std::is_convertible_v<T*, R*> { return WeakPtr<R>(static_cast<R*>(m_pointer)); }

		// T MUST be the real type spcified in MakeShared or the offset may change.
		SharedPtr<T> Pin()
		{
			if (m_pointer != nullptr)
			{
				constexpr uint32_t controlBlockSize = CONTROL_BLOCK_SIZE_AND_ALIGNMENT<T>;
				uint32_t* controlBlock = Mem::DownOffset<uint32_t>(m_pointer, controlBlockSize);
				Atomic::Increment(controlBlock);
				return SharedPtr<T>(controlBlock, m_pointer);
			}
			return SharedPtr<T>();
		}
	};

	template <typename T, typename R>
	WeakPtr<T> ReinterpretCast(WeakPtr<R> ptr)
	{
		return WeakPtr<T>(reinterpret_cast<T*>(ptr.Get()));
	}
}

namespace eastl
{
	template <typename T>
	class hash<glex::WeakPtr<T>>
	{
	public:
		size_t operator()(glex::WeakPtr<T> ptr) const
		{
			return eastl::hash<T*>()(ptr.Get());
		}
	};

	template <typename T>
	class hash<glex::UniquePtr<T>>
	{
	public:
		size_t operator()(glex::WeakPtr<T> ptr) const
		{
			return eastl::hash<T*>()(ptr.Get());
		}
	};
}