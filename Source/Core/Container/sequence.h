#pragma once
#include "Core/Container/basic.h"
#include <initializer_list>

namespace glex
{
	template <typename T>
	class SequenceView
	{
	private:
		friend class SequenceView<T const>;
		T* m_buffer;
		uint32_t m_size;

	public:
		// Reference to array decays to pointer in template argument deduction...
		SequenceView(nullptr_t rhs) : m_buffer(nullptr), m_size(0) {}
		SequenceView() : m_buffer(nullptr), m_size(0) {}
		SequenceView(T* buffer) : m_buffer(buffer), m_size(1) {}
		SequenceView(std::pair<T*, uint32_t> array) : m_buffer(array.first), m_size(array.second) {}
		SequenceView(T* buffer, uint32_t size) : m_buffer(buffer), m_size(size) {}
		SequenceView& operator=(nullptr_t rhs) { m_buffer = nullptr; m_size = 0; return *this; }
		SequenceView& operator=(std::pair<T*, uint32_t> buffer) { m_buffer = buffer.first; m_size = buffer.second; return *this; }
		SequenceView& operator=(T* buffer) { m_buffer = buffer; m_size = 1; return *this; }
		template <uint32_t SIZE> SequenceView(std::array<T, SIZE>& array) : m_buffer(array.data()), m_size(array.size()) {}
		SequenceView(Vector<T>& vector) : m_buffer(vector.data()), m_size(vector.size()) {}
		template <uint32_t SIZE> SequenceView& operator=(std::array<T, SIZE> const& array) { m_buffer = array.data(); m_size = array.size(); }
		SequenceView& operator=(Vector<T>& vector) { m_buffer = vector.data(); m_size = vector.size(); return *this; }
		T* Data() const { return m_buffer; }
		uint32_t Size() const { return m_size; }
		T& operator[](uint32_t index) const { return m_buffer[index]; }
		T* begin() const { return m_buffer; }
		T* end() const { return m_buffer + m_size; }

		bool operator==(SequenceView<T> rhs) const
		{
			if (m_size != rhs.m_size)
				return false;
			if (m_buffer == rhs.m_buffer)
				return true;
			for (uint32_t i = 0; i < m_size; i++)
			{
				if (m_buffer[i] != rhs.m_buffer[i])
					return false;
			}
			return true;
		}

		bool operator==(SequenceView<T const> rhs) const
		{
			if (m_size != rhs.m_size)
				return false;
			if (m_buffer == rhs.m_buffer)
				return true;
			for (uint32_t i = 0; i < m_size; i++)
			{
				if (m_buffer[i] != rhs.m_buffer[i])
					return false;
			}
			return true;
		}
	};

	template <typename T>
	class SequenceView<T const>
	{
	private:
		T const* m_buffer;
		uint32_t m_size;

	public:
		SequenceView(nullptr_t rhs) : m_buffer(nullptr), m_size(0) {}
		SequenceView() : m_buffer(nullptr), m_size(0) {}
		SequenceView(SequenceView<T> view) : m_buffer(view.m_buffer), m_size(view.m_size) {}
		SequenceView(std::pair<T const*, uint32_t> array) : m_buffer(array.first), m_size(array.second) {}
		SequenceView(T const* buffer, uint32_t size) : m_buffer(buffer), m_size(size) {}
		SequenceView(T const* buffer) : m_buffer(buffer), m_size(1) {}
		SequenceView& operator=(nullptr_t rhs) { m_buffer = nullptr; m_size = 0; return *this; }
		SequenceView& operator=(std::pair<T const*, uint32_t> buffer) { m_buffer = buffer.first; m_size = buffer.second; return *this; }
		SequenceView& operator=(T const* buffer) { m_buffer = buffer; m_size = 1; return *this; }
		template <uint32_t SIZE> SequenceView(std::array<T, SIZE> const& array) : m_buffer(array.data()), m_size(array.size()) {}
		template <uint32_t SIZE> SequenceView(std::array<T const, SIZE> const& array) : m_buffer(array.data()), m_size(array.size()) {}
		SequenceView(Vector<T> const& vector) : m_buffer(vector.data()), m_size(vector.size()) {}
		template <uint32_t SIZE> SequenceView& operator=(std::array<T, SIZE> const& array) { m_buffer = array.data(); m_size = array.size(); return *this; }
		template <uint32_t SIZE> SequenceView& operator=(std::array<T const, SIZE> const& array) { m_buffer = array.data(); m_size = array.size(); return *this; }
		SequenceView& operator=(Vector<T> const& vector) { m_buffer = vector.data(); m_size = vector.size(); return *this; }
		SequenceView(Vector<T const> const& vector) : m_buffer(vector.data()), m_size(vector.size()) {}
		SequenceView& operator=(Vector<T const> const& vector) { m_buffer = vector.data(); m_size = vector.size(); return *this; }
		SequenceView(std::initializer_list<T> const& list) : m_buffer(list.begin()), m_size(list.size()) {}
		SequenceView& operator=(std::initializer_list<T> const& list) { m_buffer = list.begin(), m_size = list.size(); return *this; }
		T const* Data() const { return m_buffer; }
		uint32_t Size() const { return m_size; }
		T const& operator[](uint32_t index) const { return m_buffer[index]; }
		T const* begin() const { return m_buffer; }
		T const* end() const { return m_buffer + m_size; }

		bool operator==(SequenceView<T> rhs) const
		{
			if (m_size != rhs.m_size)
				return false;
			if (m_buffer == rhs.m_buffer)
				return true;
			for (uint32_t i = 0; i < m_size; i++)
			{
				if (m_buffer[i] != rhs.m_buffer[i])
					return false;
			}
			return true;
		}

		bool operator==(SequenceView<T const> rhs) const
		{
			if (m_size != rhs.m_size)
				return false;
			if (m_buffer == rhs.m_buffer)
				return true;
			for (uint32_t i = 0; i < m_size; i++)
			{
				if (m_buffer[i] != rhs.m_buffer[i])
					return false;
			}
			return true;
		}
	};
}

template <typename T>
class eastl::hash<glex::SequenceView<T>>
{
public:
	size_t operator()(glex::SequenceView<T> const& seq) const
	{
		uint64_t hash = 0;
		eastl::hash<std::remove_const_t<T>> elemHasher;
		for (T const& elem : seq)
			hash = hash * 4397 + elemHasher(elem) * 3;
		return static_cast<uint32_t>(hash);
	}
};