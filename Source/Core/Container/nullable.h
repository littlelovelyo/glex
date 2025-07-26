#pragma once
#include "Core/Container/optional.h"

namespace glex
{
	template <typename T>
	class Nullable
	{
	private:
		bool m_hasValue;
		Optional<T> m_value;

	public:
		Nullable() : m_hasValue(false) {}
		Nullable(nullptr_t value) : m_hasValue(false) {}
		Nullable(T const& value) : m_hasValue(true) { m_value.Emplace(value); }
		Nullable(T&& value) : m_hasValue(true) { m_value.Emplace(std::move(value)); }

		Nullable(Nullable<T> const& rhs) : m_hasValue(rhs.m_hasValue)
		{
			if (m_hasValue)
				m_value.Emplace(rhs.m_value);
		}

		Nullable(Nullable<T>&& rhs) : m_hasValue(rhs.m_hasValue)
		{
			if (m_hasValue)
				m_value.Emplace(std::move(rhs.m_value));
			rhs.m_hasValue = false;
		}

		Nullable<T>& operator=(Nullable<T> const& rhs)
		{
			if (m_hasValue)
				m_value.Destroy();
			m_hasValue = rhs.m_hasValue;
			if (m_hasValue)
				m_value.Emplace(rhs.m_value);
			return *this;
		}

		Nullable<T>& operator=(Nullable<T>&& rhs)
		{
			if (m_hasValue)
				m_value.Destroy();
			m_hasValue = rhs.m_hasValue;
			if (m_hasValue)
				m_value.Emplace(std::move(rhs.m_value));
			rhs.m_hasValue = false;
			return *this;
		}

		bool operator==(nullptr_t rhs) const { return !m_hasValue; }
		Nullable<T>& operator*() { return *m_value; }
		Nullable<T> const& operator*() const { return *m_value; }
		T* operator->() { return m_value.operator->(); }
		T const* operator->() const { return m_value.operator->(); }
	};

	template <>
	class Nullable<bool>
	{
	private:
		int8_t m_value;

	public:
		Nullable() : m_value(-1) {}
		Nullable(nullptr_t value) : m_value(-1) {}
		Nullable(bool value) : m_value(value) {}
		Nullable(Nullable<bool> const& rhs) = default;
		Nullable<bool>& operator=(Nullable<bool> const& rhs) = default;

		Nullable(Nullable<bool>&& rhs) : m_value(rhs.m_value)
		{
			rhs.m_value = -1;
		}

		Nullable<bool>& operator=(Nullable<bool>&& rhs)
		{
			m_value = rhs.m_value;
			rhs.m_value = -1;
		}

		bool operator==(nullptr_t rhs) const { return m_value == -1; }
		bool operator*() const { return m_value; }
	};

	template <>
	class Nullable<void>
	{
	private:
		bool m_hasValue;

	public:
		Nullable() : m_hasValue(false) {}
		Nullable(nullptr_t value) : m_hasValue(false) {}
		Nullable(bool hasValue) : m_hasValue(hasValue) {}
		Nullable(Nullable<void> const& rhs) = default;
		Nullable<void>& operator=(Nullable<void> const& rhs) = default;

		Nullable(Nullable<void>&& rhs) : m_hasValue(rhs.m_hasValue)
		{
			rhs.m_hasValue = false;
		}

		Nullable<void>& operator=(Nullable<void>&& rhs)
		{
			m_hasValue = rhs.m_hasValue;
			rhs.m_hasValue = false;
			return *this;
		}

		bool operator==(nullptr_t rhs) const { return !m_hasValue; }
		void operator*() const {}
	};
}