#pragma once
#include "Core/Memory/allocator.h"
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/deque.h>
#include <EASTL/hash_set.h>
#include <EASTL/hash_map.h>
#include <EASTL/stack.h>
#include <EASTL/queue.h>
#include <EASTL/string.h>

namespace glex
{
	template <typename T>
	using Vector = eastl::vector<T, Allocator>;

	template <typename T, uint32_t InlineSize>
	using InlineVector = eastl::fixed_vector<T, InlineSize, true, Allocator>;

	template <typename T, uint32_t ArraySize = DEQUE_DEFAULT_SUBARRAY_SIZE(T)>
	using Deque = eastl::deque<T, Allocator, ArraySize>;

	template <typename T>
	using Stack = eastl::stack<T, Vector<T>>;

	template <typename T>
	using Queue = eastl::queue<T, Deque<T>>;

	template <typename K, typename V, typename Hasher = eastl::hash<K>, typename Comparator = eastl::equal_to<K>>
	using HashMap = eastl::hash_map<K, V, Hasher, Comparator, Allocator>;

	template <typename K, typename Hasher = eastl::hash<K>>
	using HashSet = eastl::hash_set<K, Hasher, eastl::equal_to<K>, Allocator>;

	using String = eastl::basic_string<char, Allocator>;
	using WideString = eastl::basic_string<wchar_t, Allocator>;

	using StringView = eastl::string_view;
}

template <>
class eastl::hash<glex::String>
{
public:
	size_t operator()(glex::String const& string) const
	{
		return eastl::hash<char const*>()(string.c_str());
	}
};

template <>
class eastl::hash<glex::WideString>
{
public:
	size_t operator()(glex::WideString const& string) const
	{
		return eastl::hash<wchar_t const*>()(string.c_str());
	}
};