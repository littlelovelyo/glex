#pragma once
#include <stdint.h>
#include <utility>
#define PY_SSIZE_T_CLEAN
#include <Py/Python.h>

namespace glex
{
	enum class PyStatus : uint8_t
	{
		RaiseException,
		RelayException,
		Success
	};

	// Error type.
	template <typename T>
	struct PyRetVal
	{
		PyStatus status;
		T ret;

		template <typename R>
		PyRetVal(PyStatus errorState, R&& returnValue) : status(errorState), ret(std::forward<R>(returnValue)) {}
	};

	template <>
	struct PyRetVal<void>
	{
		PyStatus status;

		PyRetVal(PyStatus errorState) : status(errorState) {}
	};

	// Keyword parameter wrapper.
	class PyKeywordParameters
	{
	private:
		PyObject* m_object;

	public:
		class Iterator
		{
		private:
			PyObject* m_dict;
			Py_ssize_t m_pos;
			char const* m_key;
			PyObject* m_value;

		public:
			Iterator() : m_dict(nullptr), m_pos(-1) {}
			Iterator(PyObject* dict) : m_dict(dict), m_pos(0) { ++(*this); }
			Iterator& operator++();
			bool operator==(Iterator const& rhs) const { return m_pos == rhs.m_pos; }
			std::pair<char const*, PyObject*> operator*() const { return { m_key, m_value }; }
		};

		PyKeywordParameters(PyObject* kwds) : m_object(kwds) {}
		Iterator begin() const { return m_object != nullptr ? Iterator(m_object) : Iterator(); }
		Iterator end() const { return Iterator(); }
	};
}