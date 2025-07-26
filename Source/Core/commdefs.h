#pragma once
#include <stdint.h>
#include <type_traits>
#include <glm/glm.hpp>

#define GLEX_LIKELY [[likely]]
#define GLEX_UNLIKELY [[unlikely]]

namespace glex
{
	template <typename T = void>
	constexpr T* INVALID_POINTER = reinterpret_cast<T*>(UINT64_MAX);

	namespace inner
	{
		template <typename Func>
		struct FunctionPtr
		{
		};

		template <typename Ret, typename... Args>
		struct FunctionPtr<Ret(Args...)>
		{
			using Type = Ret(*)(Args...);
		};

		template <typename Obj, typename Func>
		struct MemberFunctionPtr
		{
		};

		template <typename Obj, typename Ret, typename... Args>
		struct MemberFunctionPtr<Obj, Ret(Args...)>
		{
			using Type = Ret(Obj::*)(Args...);
		};
	}

	// Helper alias to make function pointer definition a bit easier.
	template <typename Func>
	using FunctionPtr = inner::FunctionPtr<Func>::Type;

	template <typename Obj, typename Func>
	using MemberFunctionPtr = inner::MemberFunctionPtr<Obj, Func>::Type;

	class StaticClass
	{
	private:
		StaticClass() = delete;
	};

	class Uncopyable
	{
	private:
		Uncopyable(Uncopyable const& rhs) = delete;
		Uncopyable& operator=(Uncopyable const& rhs) = delete;
	public:
		Uncopyable() = default;
		Uncopyable(Uncopyable&& rhs) = default;
		Uncopyable& operator=(Uncopyable&& rhs) = default;
	};

	class Unmoveable
	{
	private:
		Unmoveable(Unmoveable&& rhs) = delete;
		Unmoveable& operator=(Unmoveable&& rhs) = delete;
	public:
		Unmoveable() = default;
	};

	namespace concepts
	{
		template <typename T>
		concept EnumClass = std::is_scoped_enum_v<T>;

		template <typename T, uint32_t size>
		concept SizeIs = sizeof(T) == size;

		template <typename T, uint32_t size>
		concept CanFitInto = sizeof(T) <= size && alignof(T) <= size;

		template <typename To, typename From>
		concept InplaceCastable = reinterpret_cast<uint64_t>(static_cast<To*>(reinterpret_cast<From*>(0x7ff))) == 0x7ff;

		template <typename T>
		concept TrivialEmpty = std::is_empty_v<T> && std::is_trivially_constructible_v<T>;
	}

	template <typename T, uint32_t N>
	std::array<T, N>& ArrayOf(T(&array)[N])
	{
		return reinterpret_cast<std::array<T, N>&>(array);
	}

	template <typename T, uint32_t N>
	std::array<T, N> const& ArrayOf(T const(&array)[N])
	{
		return reinterpret_cast<std::array<T, N> const&>(array);
	}

	template <concepts::EnumClass EnumType>
	inline constexpr std::underlying_type_t<EnumType> operator*(EnumType value)
	{
		return static_cast<std::underlying_type_t<EnumType>>(value);
	}

	template <concepts::EnumClass EnumType>
	inline constexpr EnumType operator|(EnumType lhs, EnumType rhs)
	{
		using UnderlyingType = std::underlying_type_t<EnumType>;
		return static_cast<EnumType>(static_cast<UnderlyingType>(lhs) | static_cast<UnderlyingType>(rhs));
	}

	template <concepts::EnumClass EnumType>
	inline constexpr EnumType operator&(EnumType lhs, EnumType rhs)
	{
		using UnderlyingType = std::underlying_type_t<EnumType>;
		return static_cast<EnumType>(static_cast<UnderlyingType>(lhs) & static_cast<UnderlyingType>(rhs));
	}

	template <concepts::EnumClass EnumType>
	inline constexpr EnumType operator~(EnumType lhs)
	{
		using UnderlyingType = std::underlying_type_t<EnumType>;
		return static_cast<EnumType>(~static_cast<UnderlyingType>(lhs));
	}

	template <typename T>
	constexpr inline T const& Max(T const& l, T const& r)
	{
		return l < r ? r : l;
	}

	template <typename T, typename... Rest>
	constexpr inline T const& Max(T const& l, Rest const&... r)
	{
		return Max(l, Max(r...));
	}

	template <typename T>
	constexpr inline T const& Min(T const& l, T const& r)
	{
		return l < r ? l : r;
	}

	template <typename T, typename... Rest>
	constexpr inline T const& Min(T const& l, Rest const&... r)
	{
		return Min(l, Min(r...));
	}

	constexpr uint64_t operator""_kib(unsigned long long kib)
	{
		return kib * 1024;
	}

	constexpr uint64_t operator""_mib(unsigned long long mib)
	{
		return mib * 1024 * 1024;
	}

	constexpr uint64_t operator""_gib(unsigned long long gib)
	{
		return gib * 1024 * 1024 * 1024;
	}
}