#pragma once
#include "config.h"
#include "Core/Container/basic.h"
#include "Core/assert.h"
#include "Engine/Scripting/template.h"

namespace glex
{
	namespace py
	{
		struct BasicType
		{
			PyObject_HEAD
		};

		using MethodList = Vector<PyMethodDef>;
		using PropertyList = Vector<PyGetSetDef>;

#if GLEX_REPORT_MEMORY_LEAKS
		void WatchLists(MethodList& methodList, PropertyList& propertyList);
#endif
	}

	template <typename T>
	class Type : public py::BasicType
	{
	private:
		friend class LibraryInfo;

		inline static PyTypeObject s_typeDef =
		{
			.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
			.tp_name = nullptr,
			.tp_basicsize = 0,
			.tp_itemsize = 0,
			.tp_flags = Py_TPFLAGS_DEFAULT,
			.tp_doc = nullptr,
			.tp_methods = nullptr,
			.tp_new = PyType_GenericNew
		};
		inline static py::MethodList s_methods;
		inline static py::PropertyList s_properties;

		static PyTypeObject* Init(char const* name)
		{
			GLEX_DEBUG_ASSERT(s_typeDef.tp_name == nullptr) {}
			s_typeDef.tp_name = name;
			s_typeDef.tp_basicsize = sizeof(Type<T>);
			if (!s_methods.empty())
			{
				s_methods.emplace_back(nullptr, nullptr, 0, nullptr);
				s_typeDef.tp_methods = s_methods.data();
			}
			if (!s_properties.empty())
			{
				s_properties.emplace_back(nullptr, nullptr, nullptr, nullptr);
				s_typeDef.tp_getset = s_properties.data();
			}
#if GLEX_REPORT_MEMORY_LEAKS
			py::WatchLists(s_methods, s_properties);
#endif
			if constexpr (!std::is_trivially_constructible_v<T>)
			{
				s_typeDef.tp_new = [](PyTypeObject* type, PyObject* args, PyObject* kwds)
				{
					Type<T>* object = reinterpret_cast<Type<T>*>(type->tp_alloc(type, 0));
					new(&object->Get()) T();
					return reinterpret_cast<PyObject*>(object);
				};
			}
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				s_typeDef.tp_dealloc = [](PyObject* self)
				{
					(*reinterpret_cast<Type<T>*>(self))->~T();
				};
			}
			PyType_Ready(&s_typeDef);
			return &s_typeDef;
		}

		T m_value;

	public:
		typedef PyObject* (*PyMethod)(Type<T>*, PyObject*);
		typedef PyObject* (*PyMethodFast)(Type<T>*, PyObject* const*, Py_ssize_t);
		typedef PyObject* (*PyMethodNoArgs)(Type<T>*);
		typedef PyObject* (*Getter)(PyObject*, void*);
		typedef int (*Setter)(PyObject*, PyObject*, void*);
		typedef PyObject* (*PyGetter)(Type<T>*);
		typedef int (*PySetter)(Type<T>*, PyObject*);

		static void EnableInheritance()
		{
			s_typeDef.tp_flags |= Py_TPFLAGS_BASETYPE;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Old-fashioned style.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		static void RegisterMethod(char const* name, PyMethod method)
		{
			s_methods.emplace_back(name, reinterpret_cast<PyCFunction>(method), METH_VARARGS, nullptr);
		}

		static void RegisterMethod(char const* name, PyMethodFast method)
		{
			s_methods.emplace_back(name, reinterpret_cast<PyCFunction>(method), METH_FASTCALL, nullptr);
		}

		static void RegisterMethod(char const* name, PyMethodNoArgs method)
		{
			s_methods.emplace_back(name, reinterpret_cast<PyCFunction>(method), METH_NOARGS, nullptr);
		}

		static void RegisterProperty(char const* name, PyGetter getter, PySetter setter)
		{
			s_properties.emplace_back(name, reinterpret_cast<Getter>(getter), reinterpret_cast<Setter>(setter), nullptr, nullptr);
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Template version.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <auto MemFn>
		static void RegisterMethod(char const* name)
		{
			using MethType = py::PyMeth<decltype(MemFn)>;
			s_methods.emplace_back(name, reinterpret_cast<PyCFunction>(MethType::template Meth<MemFn>), MethType::FLAG, nullptr);
		}

		template <auto Getter, auto Setter>
		static void RegisterProperty(char const* name)
		{
			getter g;
			setter s;
			if constexpr (Getter == nullptr)
				g = nullptr;
			else
				g = reinterpret_cast<getter>(py::PyGetter<decltype(Getter)>::template Get<Getter>);
			if constexpr (Setter == nullptr)
				s = nullptr;
			else
				s = reinterpret_cast<setter>(py::PySetter<decltype(Setter)>::template Set<Setter>);
			s_properties.emplace_back(name, g, s, nullptr, nullptr);
		}

		template <auto MemFn>
		static void RegisterInit()
		{
			s_typeDef.tp_init = reinterpret_cast<initproc>(py::PyInit<decltype(MemFn)>::template Init<MemFn>);
		}

		template <auto MemFn>
		static void RegisterDel()
		{
			s_typeDef.tp_del = reinterpret_cast<destructor>(py::PyDel<decltype(MemFn)>::template Del<MemFn>);
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Helper functions.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		static Type<T>* New()
		{
			Type<T>* obj = PyObject_New(Type<T>, &s_typeDef);
			// Workaround.
			if constexpr (!std::is_trivially_constructible_v<T>)
				new(&obj->Get()) T();
			return obj;
		}

		static bool IsSameType(PyObject* obj)
		{
			PyTypeObject* typeObject = obj->ob_type;
			return typeObject == &s_typeDef;
		}

		static bool IsSameOrSubType(PyObject* obj)
		{
			return PyObject_TypeCheck(obj, &s_typeDef);
		}

		static Type<T>* Cast(PyObject* obj)
		{
			return IsSameType(obj) ? reinterpret_cast<Type<T>*>(obj) : nullptr;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Instance functions.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		T& Get()
		{
			return m_value;
		}

		T const& Get() const
		{
			return m_value;
		}

		T* operator->()
		{
			return &m_value;
		}

		T const* operator->() const
		{
			return &m_value;
		}
	};
}