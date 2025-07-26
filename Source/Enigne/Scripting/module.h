/**
 * C++-to-Python wrapper.
 */
#pragma once
#include <Py/Python.h>
#include "Core/Container/optional.h"
#include "Engine/Scripting/template.h"
#include "config.h"

namespace glex
{
	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			DUMMY DEFS
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	class PyModule;

	template <typename Fn>
	class PyFunction;

	template <typename T>
	class PyResult
	{
	private:
		PyObject* m_object = nullptr;
		T m_value = {};

	public:
		PyResult() = default;

		PyResult(PyObject* object)
		{
			if (object == nullptr)
			{
#if GLEX_REPORT_SCRIPT_ERRORS
				PyErr_Print();
#else
				PyErr_Clear();
#endif
			}
			else if (py::PyParse(object, &m_value))
				m_object = object;
			else
				Py_DECREF(object);
		}

		~PyResult()
		{
			Py_XDECREF(m_object);
		}

		PyResult(PyResult<T> const& rhs) : m_object(rhs.m_object)
		{
			if (m_object)
			{
				m_value = rhs.m_value;
				Py_INCREF(m_object);
			}
		}

		PyResult(PyResult<T>&& rhs) : m_object(rhs.m_object)
		{
			if (m_object)
			{
				m_value = std::move(rhs.m_value);
				rhs.m_object = nullptr;
			}
		}

		PyResult<T>& operator=(PyResult<T> const& rhs)
		{
			m_object = rhs.m_object;
			if (m_object)
			{
				m_value = rhs.m_value;
				Py_INCREF(m_object);
			}
			return *this;
		}

		PyResult<T>& operator=(PyResult<T>&& rhs)
		{
			m_object = rhs.m_object;
			if (m_object)
			{
				m_value = std::move(rhs.m_value);
				rhs.m_object = nullptr;
			}
			return *this;
		}

		bool HasValue() const { return m_object != nullptr; }
		T const& Value() const { return m_value; }
		T& Value() { return m_value; }
	};

	template <>
	class PyResult<void>
	{
	private:
		PyObject* m_object = nullptr;

	public:
		PyResult() = default;

		PyResult(PyObject* object)
		{
			if (object == nullptr)
			{
#if GLEX_REPORT_SCRIPT_ERRORS
				PyErr_Print();
#else
				PyErr_Clear();
#endif
			}
			else
				m_object = object;
		}

		~PyResult()
		{
			Py_XDECREF(m_object);
		}

		PyResult(PyResult<void> const& rhs) : m_object(rhs.m_object)
		{
			Py_XINCREF(m_object);
		}

		PyResult(PyResult<void>&& rhs) : m_object(rhs.m_object)
		{
			rhs.m_object = nullptr;
		}

		PyResult<void>& operator=(PyResult<void> const& rhs)
		{
			m_object = rhs.m_object;
			Py_XINCREF(m_object);
			return *this;
		}

		PyResult<void>& operator=(PyResult<void>&& rhs)
		{
			m_object = rhs.m_object;
			rhs.m_object = nullptr;
			return *this;
		}

		bool HasValue() const { return m_object != nullptr; }
		void Value() const {}
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			GENERAL CASE

			Do not use char const* as return type. Use String instead.
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename Ret, typename... Args>
	class PyFunction<Ret(Args...)>
	{
		friend class PyModule;

	private:
		PyObject* m_function;

		PyFunction(PyObject* obj) : m_function(obj) {}

	public:
		PyFunction() : m_function(nullptr) {}
		PyFunction(PyFunction const& rhs) : m_function(rhs.m_function) { Py_XINCREF(m_function); }
		PyFunction& operator=(PyFunction const& rhs) { m_function = rhs.m_function; Py_XINCREF(m_function); return *this; }
		PyFunction(PyFunction&& rhs) : m_function(rhs.m_function) { rhs.m_function = nullptr; }
		PyFunction& operator=(PyFunction&& rhs) { std::swap(m_function, rhs.m_function); return *this; }
		PyFunction& operator=(nullptr_t rhs) { Py_XDECREF(m_function); m_function = nullptr; return *this; }
		~PyFunction() { Py_XDECREF(m_function); }
		bool IsValid() const { return m_function != nullptr; }
		bool operator==(nullptr_t rhs) const { return m_function == nullptr; }

		PyResult<Ret> operator()(Args... args) const
		{
			PyObject* tuple = py::PyReturn(std::tuple<Args const&...> { args... });
			PyObject* pyRet = PyObject_Call(m_function, tuple, nullptr);
			Py_XDECREF(tuple);
			return PyResult<Ret>(pyRet);
		}
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			NO ARGS SPECIALIZATION
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename Ret>
	class PyFunction<Ret()>
	{
		friend class PyModule;

	private:
		PyObject* m_function;

		PyFunction(PyObject* obj) : m_function(obj) {}

	public:
		PyFunction() : m_function(nullptr) {}
		PyFunction(PyFunction const& rhs) : m_function(rhs.m_function) { Py_XINCREF(m_function); }
		PyFunction& operator=(PyFunction const& rhs) { m_function = rhs.m_function; Py_XINCREF(m_function); return *this; }
		PyFunction(PyFunction&& rhs) : m_function(rhs.m_function) { rhs.m_function = nullptr; }
		PyFunction& operator=(PyFunction&& rhs) { std::swap(m_function, rhs.m_function); return *this; }
		PyFunction& operator=(nullptr_t rhs) { Py_XDECREF(m_function); m_function = nullptr; return *this; }
		~PyFunction() { Py_XDECREF(m_function); }
		bool IsValid() const { return m_function != nullptr; }
		bool operator==(nullptr_t rhs) const { return m_function == nullptr; }

		PyResult<Ret> operator()() const
		{
			PyObject* pyRet = PyObject_CallNoArgs(m_function);
			return PyResult<Ret>(pyRet);
		}
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			ONE ARG SPECIALIZATION
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	template <typename Ret, typename Arg>
	class PyFunction<Ret(Arg)>
	{
		friend class PyModule;

	private:
		PyObject* m_function;

		PyFunction(PyObject* obj) : m_function(obj) {}

	public:
		PyFunction() : m_function(nullptr) {}
		PyFunction(PyFunction const& rhs) : m_function(rhs.m_function) { Py_XINCREF(m_function); }
		PyFunction& operator=(PyFunction const& rhs) { m_function = rhs.m_function; Py_XINCREF(m_function); return *this; }
		PyFunction(PyFunction&& rhs) : m_function(rhs.m_function) { rhs.m_function = nullptr; }
		PyFunction& operator=(PyFunction&& rhs) { std::swap(m_function, rhs.m_function); return *this; }
		PyFunction& operator=(nullptr_t rhs) { Py_XDECREF(m_function); m_function = nullptr; return *this; }
		~PyFunction() { Py_XDECREF(m_function); }
		bool IsValid() const { return m_function != nullptr; }
		bool operator==(nullptr_t rhs) const { return m_function == nullptr; }

		PyResult<Ret> operator()(Arg arg) const
		{
			PyObject* xarg = py::PyReturn(arg);
			PyObject* pyRet = PyObject_CallOneArg(m_function, xarg);
			Py_XDECREF(xarg);
			return PyResult<Ret>(pyRet);
		}
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			MODULE
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	class PyModule
	{
	private:
		PyObject* m_module;

	public:
		PyModule() : m_module(nullptr) {}
		PyModule(PyModule const& rhs) : m_module(rhs.m_module) { Py_XINCREF(m_module); }
		PyModule& operator=(PyModule const& rhs) { m_module = rhs.m_module; Py_XINCREF(m_module); return *this; }
		PyModule(PyModule&& rhs) : m_module(rhs.m_module) { rhs.m_module = nullptr; }
		PyModule& operator=(PyModule&& rhs) { std::swap(m_module, rhs.m_module); return *this; }
		PyModule& operator=(nullptr_t rhs) { Py_XDECREF(m_module); m_module = nullptr; return *this; }

		PyModule(char const* name);
		~PyModule();
		bool IsValid() const { return m_module != nullptr; }
		bool operator==(nullptr_t rhs) const { return m_module == nullptr; }
		PyObject* GetObject(char const* name);

		template <typename Fn>
		PyFunction<Fn> GetFunction(char const* name)
		{
			PyObject* obj = GetObject(name);
			if (!PyCallable_Check(obj))
			{
				Py_DECREF(obj);
				obj = nullptr;
			}
			return PyFunction<Fn>(obj);
		}
	};
}