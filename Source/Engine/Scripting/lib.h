/**
 * A library is a Python module that is implemented in C++.
 * glex provides a 'standard' module, you can define you own one.
 * Python module must be registered before application starts.
 */
#pragma once
#include "Engine/Scripting/type.h"

namespace glex
{
	class LibraryInfo
	{
	public:
		struct Class
		{
			char const* name;
			PyTypeObject* (*desc)(char const* name);
		};
		using ClassList = Vector<Class>;

	private:
		char const* m_name;
		mutable py::MethodList m_methods;
		mutable ClassList m_classes;

#if GLEX_REPORT_MEMORY_LEAKS
		inline static Vector<py::MethodList*> s_methodLists;
		inline static Vector<py::PropertyList*> s_propertyLists;
#endif

	public:
		typedef PyObject* (*PyCFunctionNoArgs)(PyObject* self);

		LibraryInfo(char const* name, uint32_t numFunctions, uint32_t numClasses) : m_name(name)
		{
			m_methods.reserve(numFunctions);
			m_classes.reserve(numClasses);
		}

		char const* Name() const
		{
			return m_name;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Plain method.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		void Register(char const* fn, _PyCFunctionFast impl)
		{
			m_methods.emplace_back(fn, reinterpret_cast<PyCFunction>(impl), METH_FASTCALL, nullptr);
		}

		void Register(char const* fn, PyCFunction impl)
		{
			m_methods.emplace_back(fn, impl, METH_VARARGS, nullptr);
		}

		void Register(char const* fn, PyCFunctionNoArgs impl)
		{
			m_methods.emplace_back(fn, reinterpret_cast<PyCFunction>(impl), METH_NOARGS, nullptr);
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Templated method.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <auto NativeFunction>
		void Register(char const* name)
		{
			using FuncType = py::PyFunc<decltype(NativeFunction)>;
			m_methods.emplace_back(name, reinterpret_cast<PyCFunction>(FuncType::template Func<NativeFunction>), FuncType::FLAG, nullptr);
		}

		template <typename T>
		void Register(char const* name)
		{
			m_classes.emplace_back(name, Type<T>::Init);
		}

#ifdef GLEX_INTERNAL
		py::MethodList& GetMethodList() const
		{
			return m_methods;
		}

		ClassList& GetClassList() const
		{
			return m_classes;
		}

#if GLEX_REPORT_MEMORY_LEAKS
		friend void py::WatchLists(MethodList& methodList, PropertyList& propertyList);

		static void ForceFreeMemory()
		{
			for (py::MethodList* v : s_methodLists)
			{
				v->clear();
				v->shrink_to_fit();
			}
			for (py::PropertyList* v : s_propertyLists)
			{
				v->clear();
				v->shrink_to_fit();
			}
			s_methodLists.clear();
			s_methodLists.shrink_to_fit();
			s_propertyLists.clear();
			s_propertyLists.shrink_to_fit();
		}
#endif
#endif
	};
}