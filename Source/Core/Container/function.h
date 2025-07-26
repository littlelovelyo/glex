#pragma once
#include "Core/commdefs.h"
#include "Core/Memory/mem.h"
#include <array>

namespace glex
{
	template <typename Fn, uint32_t InlineStorage = 24>
	class Function;

	template <typename Fn>
	struct ReferenceLambda
	{
		Fn& obj;
		ReferenceLambda(Fn& o) : obj(o) {}
	};

	/**
	 * Note: function itself is a 'function', function pointer is not a 'function', function reference is not a 'function'.
	 * Reference to a function pointer is invocable.
	 * Pointers/References are trivially destructible, no matter what.
	 */
	template <typename Fn, typename... Args>
	concept FunctionObject = !std::is_function_v<std::remove_pointer_t<std::remove_reference_t<Fn>>> && std::is_invocable_v<Fn, Args...> && std::is_trivially_destructible_v<std::remove_reference_t<Fn>> && std::is_trivially_copyable_v<std::remove_reference_t<Fn>> && sizeof(Fn) < 32768;

	template <typename Ret, typename... Args, uint32_t InlineStorage>
	class Function<Ret(Args...), InlineStorage>
	{
		static_assert(InlineStorage >= 8 && InlineStorage % 8 == 0);
	private:
		/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
				FunctionPointer:
					objectSize: 0
					useExternalStorage: 0

				Inlined function object:
					objectSize: size of that object
					useExternalStorage: 0

				Heap-allocated function object:
					objectSize: size of that object
					useExternalStorage: 1

				Member function:
					objectSize: 0
					useExternalStorage: 1
		 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
		 // Bit operation may be faster than indirect call.
		uint64_t m_functionPointer : 48 = 0;
		uint64_t m_objectSize : 15 = 0; // Max 32767, should be enough.
		uint64_t m_useExternalStorage : 1 = 0; // If this is true but m_objectSize is 0, it's bound to a membmer function.
		union
		{
			void* m_objectPointer;
			mutable std::array<uint64_t, InlineStorage / 8> m_objectStorage;
		};

		using ThisType = Function<Ret(Args...), InlineStorage>;

		template <uint32_t InlineStorage>
		using Rebind = Function<Ret(Args...), InlineStorage>;

		template <typename Fn, uint32_t InlineStorage>
		friend class Function;

		void const* GetObjectPointer() const
		{
			return m_useExternalStorage ? m_objectPointer : m_objectStorage;
		}

		void FreeStorage()
		{
			if (m_useExternalStorage && m_objectSize != 0)
				Mem::Free(m_objectPointer);
		}

	public:
		/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
				Basic constructors/assign operators.
		 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
		Function() = default;

		~Function()
		{
			FreeStorage();
		}

		template <uint32_t HisInlineStorage>
		Function(Rebind<HisInlineStorage> const& rhs) : m_functionPointer(rhs.m_functionPointer), m_objectSize(rhs.m_objectSize)
		{
			if (m_objectSize > InlineStorage) // Cannot fit.
			{
				m_objectPointer = Mem::Alloc(m_objectSize, 8);
				memcpy(m_objectPointer, rhs.GetObjectPointer(), m_objectSize);
				m_useExternalStorage = true;
			}
			else if (m_objectSize > 0) // Can fit.
				memcpy(&m_objectStorage, rhs.GetObjectPointer(), m_objectSize);
			else if (rhs.m_useExternalStorage) // Member function.
			{
				m_objectPointer = rhs.m_objectPointer;
				m_useExternalStorage = true;
			}
		}

		template <uint32_t HisInlineStorage>
		Function(Rebind<HisInlineStorage>&& rhs) : m_functionPointer(rhs.m_functionPointer), m_objectSize(rhs.m_objectSize)
		{
			if (rhs.m_useExternalStorage)
			{
				// Steal the pointer.
				m_objectPointer = rhs.m_objectPointer;
				m_useExternalStorage = true;
				rhs.m_useExternalStorage = false;
			}
			else if (m_objectSize > InlineStorage) // Cannot fit.
			{
				m_objectPointer = Mem::Alloc(m_objectSize, 8);
				memcpy(m_objectPointer, &rhs.m_objectStorage, m_objectSize);
				m_useExternalStorage = true;
			}
			else if (m_objectSize > 0) // Can fit.
				memcpy(&m_objectStorage, &rhs.m_objectStorage, m_objectSize);
		}

		template <uint32_t HisInlineStorage>
		ThisType& operator=(Rebind<HisInlineStorage> const& rhs)
		{
			FreeStorage();
			m_functionPointer = rhs.m_functionPointer;
			m_objectSize = rhs.m_objectSize;
			if (m_objectSize > InlineStorage) // Cannot fit.
			{
				m_objectPointer = Mem::Alloc(m_objectSize, 8);
				memcpy(m_objectPointer, rhs.GetObjectPointer(), m_objectSize);
				m_useExternalStorage = true;
			}
			else if (m_objectSize > 0) // Can fit.
			{
				memcpy(&m_objectStorage, rhs.GetObjectPointer(), m_objectSize);
				m_useExternalStorage = false;
			}
			else if (rhs.m_useExternalStorage)
			{
				m_objectPointer = rhs.m_objectPointer;
				m_useExternalStorage = true;
			}
			return *this;
		}

		template <uint32_t HisInlineStorage>
		ThisType& operator=(Rebind<HisInlineStorage>&& rhs)
		{
			FreeStorage();
			m_functionPointer = rhs.m_functionPointer;
			m_objectSize = rhs.m_objectSize;
			if (rhs.m_useExternalStorage)
			{
				m_objectPointer = rhs.m_objectPointer;
				m_useExternalStorage = true;
				rhs.m_useExternalStorage = false;
			}
			else if (m_objectSize > InlineStorage)
			{
				m_objectPointer = Mem::Alloc(m_objectSize, 8);
				memcpy(m_objectPointer, &rhs.m_objectStorage, m_objectSize);
				m_useExternalStorage = true;
			}
			else if (m_objectSize > 0)
			{
				memcpy(&m_objectStorage, &rhs.m_objectStorage, m_objectSize);
				m_useExternalStorage = false;
			}
			return *this;
		}

		bool operator==(nullptr_t rhs) const
		{
			return m_functionPointer == 0;
		}

		// Just compare the function. We don't care if objects are the same.
		template <uint32_t HisInlineStorage>
		bool operator==(Rebind<HisInlineStorage>&& rhs) const
		{
			return m_functionPointer == rhs.m_functionPointer;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
				Naked function pointer.
		 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
		Function(FunctionPtr<Ret(Args...)> fn)
		{
			m_functionPointer = reinterpret_cast<uint64_t>(fn);
		}

		ThisType& operator=(nullptr_t rhs)
		{
			FreeStorage();
			m_functionPointer = 0;
			m_objectSize = 0;
			m_useExternalStorage = false;
			return *this;
		}

		ThisType& operator=(FunctionPtr<Ret(Args...)> fn)
		{
			FreeStorage();
			m_functionPointer = fn;
			m_objectSize = 0;
			m_useExternalStorage = false;
			return *this;
		}

		bool operator==(FunctionPtr<Ret(Args...)> fn) const
		{
			return m_functionPointer == reinterpret_cast<uint64_t>(fn);
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
				Member function.
		 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename Obj>
		Function(Obj* obj, MemberFunctionPtr<Obj, Ret(Args...)> memFn)
		{
			m_functionPointer = *reinterpret_cast<uint64_t*>(&memFn);
			m_useExternalStorage = true;
			m_objectPointer = obj;
		}

		template <typename Obj>
		ThisType& BindMember(Obj* obj, MemberFunctionPtr<Obj, Ret(Args...)> memFn)
		{
			FreeStorage();
			m_functionPointer = *reinterpret_cast<uint64_t*>(&memFn);
			m_objectSize = 0;
			m_useExternalStorage = true;
			m_objectPointer = obj;
			return *this;
		}

		template <typename Obj>
		bool operator==(MemberFunctionPtr<Obj, Ret(Args...)> memFn) const
		{
			return m_functionPointer == *reinterpret_cast<uint64_t*>(&memFn);
		}

		template <typename Obj>
		bool Equals(Obj const* obj, MemberFunctionPtr<Obj, Ret(Args...)> memFn) const
		{
			return m_objectPointer == obj && m_functionPointer == *reinterpret_cast<uint64_t*>(&memFn);
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
				Function object.
		 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
	private:
		// MSVC GENERATES a __stdcall body for a lambda when it sees a __thiscall function pointer conversion.
		template <typename Obj>
		void BindOperator(Ret(Obj::*fn)(Args...))
		{
			m_functionPointer = *reinterpret_cast<uint64_t*>(&fn);
		}

		template <typename Obj>
		void BindOperator(Ret(Obj::*fn)(Args...) const)
		{
			m_functionPointer = *reinterpret_cast<uint64_t*>(&fn);
		}

	public:
		template <FunctionObject<Args...> Fn>
		Function(Fn&& fn)
		{
			using Type = std::remove_cv_t<std::remove_reference_t<Fn>>;
			m_objectSize = sizeof(Type);
			if constexpr (sizeof(Type) <= InlineStorage && alignof(Type) <= 8)
			{
				BindOperator(&Type::operator());
				new (&m_objectStorage) Type(std::forward<Fn>(fn));
			}
			else
			{
				BindOperator(&Type::operator());
				m_useExternalStorage = true;
				m_objectPointer = Mem::New<Type>(std::forward<Fn>(fn));
			}
		}

		template <typename Fn>
		Function(ReferenceLambda<Fn> fn)
		{
			using Type = std::remove_cv_t<std::remove_reference_t<Fn>>;
			m_objectSize = 0;
			m_useExternalStorage = true;
			BindOperator(&Type::operator());
			m_objectPointer = &fn.obj;
		}

		template <FunctionObject<Args...> Fn>
		ThisType& operator=(Fn&& fn)
		{
			using Type = std::remove_cv_t<std::remove_reference_t<Fn>>;
			FreeStorage();
			m_objectSize = sizeof(Type);
			if constexpr (sizeof(Type) <= InlineStorage && alignof(Type) <= 8)
			{
				BindOperator(&Type::operator());
				m_useExternalStorage = false;
				new (&m_objectStorage) Type(std::forward<Fn>(fn));
			}
			else
			{
				BindOperator(&Type::operator());
				m_useExternalStorage = true;
				m_objectPointer = Mem::New<Type>(std::forward<Fn>(fn));
			}
			return *this;
		}

		template <typename Fn>
		ThisType& operator=(ReferenceLambda<Fn> fn)
		{
			using Type = std::remove_cv_t<std::remove_reference_t<Fn>>;
			FreeStorage();
			m_objectSize = 0;
			m_useExternalStorage = true;
			BindOperator(&Type::operator());
			m_objectPointer = &fn.obj;
		}

		template <typename Fn>
		bool operator==(Fn const& fn) const requires !std::is_function_v<std::remove_pointer_t<std::remove_reference_t<Fn>>>
		{
			using Type = std::remove_cv_t<std::remove_reference_t<Fn>>;
			auto op = &Type::operator();
			return m_functionPointer == *reinterpret_cast<uint64_t*>(&op);
		}

	private:
		// __thiscall is ignored by x64 compiler. this goes to RCX.
		class ThisCall
		{
		public:
			Ret Invoke(Args... args);
		};

		template <typename... RealArgs>
		Ret InvokeThisCall(void const* function, void* object, RealArgs&&... args) const
		{
			using MemberFunctionType = MemberFunctionPtr<ThisCall, Ret(Args...)>;
			MemberFunctionType memfn = *reinterpret_cast<MemberFunctionType*>(&function);
			return (reinterpret_cast<ThisCall*>(object)->*memfn)(std::forward<RealArgs>(args)...);
		}

	public:
		template <typename... RealArgs>
		Ret operator()(RealArgs&&... args) const
		{
			typedef Ret(*Cdecl)(Args... args);			
			if (m_useExternalStorage)
				return InvokeThisCall(reinterpret_cast<void*>(m_functionPointer), m_objectPointer, std::forward<RealArgs>(args)...);
			else if (m_objectSize > 0)
				return InvokeThisCall(reinterpret_cast<void*>(m_functionPointer), &m_objectStorage, std::forward<RealArgs>(args)...);
			else
				return reinterpret_cast<Cdecl>(m_functionPointer)(std::forward<RealArgs>(args)...);
		}
	};
}