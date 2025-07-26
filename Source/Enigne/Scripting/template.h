/**
 * Template magic for Python method wrapper.
 */
#pragma once
#include "Engine/Scripting/basic.h"
#include "Core/Container/basic.h"

namespace glex
{
	template <typename T>
	class Type;

	namespace py
	{
		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				RETURN VALUE CONVERTER
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		////////////             ////////////
		//////////// Basic types ////////////
		////////////             ////////////
		inline PyObject* PyReturn(bool value)
		{
			if (value)
			{
				Py_INCREF(Py_True);
				return Py_True;
			}
			else
			{
				Py_INCREF(Py_False);
				return Py_False;
			}
		}

		inline PyObject* PyReturn(int32_t value)
		{
			return PyLong_FromLong(value);
		}

		inline PyObject* PyReturn(uint32_t value)
		{
			return PyLong_FromUnsignedLong(value);
		}

		inline PyObject* PyReturn(int64_t value)
		{
			return PyLong_FromLongLong(value);
		}

		inline PyObject* PyReturn(uint64_t value)
		{
			return PyLong_FromUnsignedLongLong(value);
		}

		inline PyObject* PyReturn(double value)
		{
			return PyFloat_FromDouble(value);
		}

		template <concepts::EnumClass Enum>
		PyObject* PyReturn(Enum value)
		{
			return PyReturn(static_cast<std::underlying_type_t<Enum>>(value));
		}

		////////////        ////////////
		//////////// String ////////////
		////////////        ////////////
		inline PyObject* PyReturn(char const* value)
		{
			return PyUnicode_FromString(value);
		}

		inline PyObject* PyReturn(String const& value)
		{
			return PyUnicode_FromStringAndSize(value.c_str(), value.length());
		}

		////////////             ////////////
		//////////// Object type ////////////
		////////////             ////////////
		template <typename T>
		PyObject* PyReturn(Type<T>* value)
		{
			return reinterpret_cast<PyObject*>(value);
		}

		inline PyObject* PyReturn(PyObject* value)
		{
			return value;
		}

		////////////                            ////////////
		//////////// Forward declare tuple type ////////////
		////////////                            ////////////
		template <typename... Args>
		PyObject* PyReturn(std::tuple<Args...> const& tuple);

		////////////              ////////////
		//////////// Vector types ////////////
		////////////              ////////////
		inline PyObject* PyReturn(glm::vec2 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y });
		}

		inline PyObject* PyReturn(glm::vec3 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z });
		}

		inline PyObject* PyReturn(glm::vec4 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z, value.w });
		}
		inline PyObject* PyReturn(glm::ivec2 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y });
		}

		inline PyObject* PyReturn(glm::ivec3 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z });
		}

		inline PyObject* PyReturn(glm::ivec4 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z, value.w });
		}

		inline PyObject* PyReturn(glm::uvec2 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y });
		}

		inline PyObject* PyReturn(glm::uvec3 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z });
		}

		inline PyObject* PyReturn(glm::uvec4 const& value)
		{
			return PyReturn(std::tuple { value.x, value.y, value.z, value.w });
		}

		inline PyObject* PyReturn(glm::mat4 const& value)
		{
			return PyReturn(std::tuple { value[0], value[1], value[2], value[3] });
		}

		////////////                              ////////////
		//////////// Aggregate type (pair, tuple) ////////////
		////////////                              ////////////
		template <typename T, typename K>
		PyObject* PyReturn(std::pair<T, K> const& pair)
		{
			PyObject* tuple = PyTuple_New(2);
			PyTuple_SetItem(tuple, 0, PyReturn(pair.first));
			PyTuple_SetItem(tuple, 1, PyReturn(pair.second));
			return tuple;
		}

		template <typename... Args, uint32_t... Indices>
		void FillTuple(PyObject* pyTuple, std::tuple<Args...> const& tuple, std::integer_sequence<uint32_t, Indices...> indices)
		{
			std::initializer_list { (PyTuple_SetItem(pyTuple, Indices, PyReturn(std::get<Indices>(tuple))), 0)... };
		}

		template <typename... Args>
		PyObject* PyReturn(std::tuple<Args...> const& tuple)
		{
			PyObject* pyTuple = PyTuple_New(sizeof...(Args));
			FillTuple(pyTuple, tuple, std::make_integer_sequence<uint32_t, sizeof...(Args)>());
			return pyTuple;
		}

		////////////               ////////////
		//////////// PyRetVal type ////////////
		////////////               ////////////
		template <typename T>
		PyObject* PyReturn(PyRetVal<T> error)
		{
			if (error.status == PyStatus::Success)
				return PyReturn(std::move(error.ret));
			if (error.status == PyStatus::RaiseException)
				PyErr_SetNone(PyExc_RuntimeError);
			return nullptr;
		}

		inline PyObject* PyReturn(PyRetVal<void> error)
		{
			if (error.status == PyStatus::Success)
			{
				Py_INCREF(Py_None);
				return Py_None;
			}
			if (error.status == PyStatus::RaiseException)
				PyErr_SetNone(PyExc_RuntimeError);
			return nullptr;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				ARGUMENT PARSER
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		////////////             ////////////
		//////////// Basic types ////////////
		////////////             ////////////
		inline int32_t PyParse(PyObject* arg, uint8_t* out)
		{
			*out = PyLong_AsUnsignedLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, int8_t* out)
		{
			*out = PyLong_AsLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, uint16_t* out)
		{
			*out = PyLong_AsUnsignedLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, int16_t* out)
		{
			*out = PyLong_AsLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, uint32_t* out)
		{
			*out = PyLong_AsUnsignedLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, int32_t* out)
		{
			*out = PyLong_AsLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, uint64_t* out)
		{
			*out = PyLong_AsUnsignedLongLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, int64_t* out)
		{
			*out = PyLong_AsLongLong(arg);
			if (*out == -1 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, double* out)
		{
			*out = PyFloat_AsDouble(arg);
			if (*out == -1.0 && PyErr_Occurred() != nullptr)
				return 0;
			return 1;
		}

		inline int32_t PyParse(PyObject* arg, float* out)
		{
			double db;
			int32_t ret = PyParse(arg, &db);
			*out = db;
			return ret;
		}

		inline int32_t PyParse(PyObject* arg, bool* out)
		{
			*out = Py_IsTrue(arg);
			return 1;
		}

		template <concepts::EnumClass Enum>
		int32_t PyParse(PyObject* arg, Enum* out)
		{
			return PyParse(arg, reinterpret_cast<std::underlying_type_t<Enum>*>(out));
		}

		////////////              ////////////
		//////////// Object types ////////////
		////////////              ////////////
		inline int32_t PyParse(PyObject* arg, PyObject** out)
		{
			*out = arg;
			return 1;
		}

		template <typename T>
		int32_t PyParse(PyObject* arg, Type<T>** out)
		{
			if (!Type<T>::IsSameOrSubType(arg))
			{
				PyErr_SetNone(PyExc_TypeError);
				return 0;
			}
			*out = reinterpret_cast<Type<T>*>(arg);
			return 1;
		}

		////////////        ////////////
		//////////// String ////////////
		////////////        ////////////
		// Parameter is cleaned by the caller so it's safe to use the string buffer at least now.
		inline int32_t PyParse(PyObject* arg, char const** out)
		{
			char const* data = PyUnicode_AsUTF8AndSize(arg, nullptr);
			*out = data;
			return data != nullptr;
		}

		inline int32_t PyParse(PyObject* arg, String* out)
		{
			Py_ssize_t size;
			char const* data = PyUnicode_AsUTF8AndSize(arg, &size);
			if (data != nullptr)
			{
				out->resize(size);
				memcpy(out->data(), data, size);
				return 1;
			}
			return 0;
		}

		////////////                 ////////////
		//////////// Forward declare ////////////
		////////////                 ////////////
		template <typename T>
		int32_t PyParse(PyObject* arg, Vector<T>* out);

		template <typename T, typename K>
		int32_t PyParse(PyObject* arg, std::pair<T, K>* out);

		template <typename... Args>
		int32_t PyParse(PyObject* arg, std::tuple<Args...>* out);

		int32_t PyParse(PyObject* arg, glm::vec2* out);
		int32_t PyParse(PyObject* arg, glm::vec3* out);
		int32_t PyParse(PyObject* arg, glm::vec4* out);
		int32_t PyParse(PyObject* arg, glm::uvec2* out);
		int32_t PyParse(PyObject* arg, glm::uvec3* out);
		int32_t PyParse(PyObject* arg, glm::uvec4* out);
		int32_t PyParse(PyObject* arg, glm::ivec2* out);
		int32_t PyParse(PyObject* arg, glm::ivec3* out);
		int32_t PyParse(PyObject* arg, glm::ivec4* out);

		// Tuples.
		template <typename T>
		int32_t PyParseTupleItem(PyObject* tuple, uint32_t index, T* out)
		{
			PyObject* item = PyTuple_GetItem(tuple, index);
			if (item == nullptr)
				return 0;
			return PyParse(item, out);
		}

		// Parse any aggregate type.
		template <typename... Args, uint32_t... Indices>
		int32_t PyParseAggregate(PyObject* arg, std::integer_sequence<uint32_t, Indices...> indices, Args*... out)
		{
			return (... && PyParseTupleItem(arg, Indices, out));
		}

		template <typename... Args>
		int32_t PyParse(PyObject* arg, Args*... out)
		{
			return PyParseAggregate(arg, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), out...);
		}

		// Vector type.
		inline int32_t PyParse(PyObject* arg, glm::vec2* out)
		{
			return PyParse(arg, &out->x, &out->y);
		}

		inline int32_t PyParse(PyObject* arg, glm::vec3* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z);
		}

		inline int32_t PyParse(PyObject* arg, glm::vec4* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z, &out->w);
		}

		inline int32_t PyParse(PyObject* arg, glm::uvec2* out)
		{
			return PyParse(arg, &out->x, &out->y);
		}

		inline int32_t PyParse(PyObject* arg, glm::uvec3* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z);
		}

		inline int32_t PyParse(PyObject* arg, glm::uvec4* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z, &out->w);
		}

		inline int32_t PyParse(PyObject* arg, glm::ivec2* out)
		{
			return PyParse(arg, &out->x, &out->y);
		}

		inline int32_t PyParse(PyObject* arg, glm::ivec3* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z);
		}

		inline int32_t PyParse(PyObject* arg, glm::ivec4* out)
		{
			return PyParse(arg, &out->x, &out->y, &out->z, &out->w);
		}

		inline int32_t PyParse(PyObject* arg, glm::mat4* out)
		{
			return PyParse(arg, &(*out)[0], &(*out)[1], &(*out)[2], &(*out)[3]);
		}

		template <typename T, typename K>
		int32_t PyParse(PyObject* arg, std::pair<T, K>* out)
		{
			return PyParseTupleItem(arg, 0, &out->first) && PyParseTupleItem(arg, 1, &out->second);
		}

		template <typename... Args, uint32_t... Indices>
		int32_t PyParseTuple(PyObject* arg, std::integer_sequence<uint32_t, Indices...> indices, std::tuple<Args...>* out)
		{
			return (... && PyParseTupleItem(arg, Indices, &std::get<Indices>(*out)));
		}

		template <typename... Args>
		int32_t PyParse(PyObject* arg, std::tuple<Args...>* out)
		{
			return PyParseTuple(arg, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), out);
		}

		// Put lists at the back. (Add ListView in the future to prevent memory allocation)
		template <typename T>
		int32_t PyParse(PyObject* arg, Vector<T>* out)
		{
			auto size = PyList_Size(arg);
			if (size == -1)
				return 0;
			out->resize(size);
			for (uint32_t i = 0; i < size; i++)
			{
				PyObject* item = PyList_GetItem(arg, i);
				if (!PyParse(item, &(*out)[i]))
					return 0;
			}
			return 1;
		}

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Python method templates.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename Fn>
		struct PyFunc {};

		////////////         ////////////
		//////////// No args ////////////
		////////////         ////////////
		template <typename Ret>
		struct PyFunc<Ret(*)()>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <Ret(*NativeImpl)()>
			static PyObject* Func(PyObject* self)
			{
				return PyReturn(NativeImpl());
			}
		};

		template <>
		struct PyFunc<void(*)()>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <void(*NativeImpl)()>
			static PyObject* Func(PyObject* self)
			{
				NativeImpl();
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Ret>
		struct PyFunc<Ret(*)(PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <Ret(*NativeImpl)(PyKeywordParameters)>
			static PyObject* Func(PyObject* self, PyObject* args, PyObject* kwds)
			{
				return PyReturn(NativeImpl(PyKeywordParameters(kwds)));
			}
		};

		template <>
		struct PyFunc<void(*)(PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <void(*NativeImpl)(PyKeywordParameters)>
			static PyObject* Func(PyObject* self, PyObject* args, PyObject* kwds)
			{
				NativeImpl(PyKeywordParameters(kwds));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		////////////           ////////////
		//////////// With args ////////////
		////////////           ////////////
		template <typename... Args, uint32_t... Indices>
		bool PyParseArgs(PyObject* const* args, std::tuple<Args...>& out, std::integer_sequence<uint32_t, Indices...> indices)
		{
			return (... && PyParse(args[Indices], &std::get<Indices>(out)));
		}

		template <typename Ret, typename... Args>
		struct PyFunc<Ret(*)(Args...)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <Ret(*NativeImpl)(Args...)>
			static PyObject* Func(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply(NativeImpl, std::move(arguments));
				return PyReturn(ret);
			}
		};

		template <typename... Args>
		struct PyFunc<void(*)(Args...)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(*NativeImpl)(Args...)>
			static PyObject* Func(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply(NativeImpl, std::move(arguments));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Ret, typename... Args>
		struct PyFunc<Ret(*)(Args..., PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL | METH_KEYWORDS;
			template <Ret(*NativeImpl)(Args..., PyKeywordParameters)>
			static PyObject* Func(PyObject* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply(NativeImpl, std::move(arguments), PyKeywordParameters(kwds));
				return PyReturn(ret);
			}
		};

		template <typename... Args>
		struct PyFunc<void(*)(Args..., PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(*NativeImpl)(Args..., PyKeywordParameters)>
			static PyObject* Func(PyObject* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply(NativeImpl, std::move(arguments), PyKeywordParameters(kwds));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				MEMBER FUNCTIONS
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename Fn>
		struct PyMeth {};

		////////////         ////////////
		//////////// No args ////////////
		////////////         ////////////
		template <typename Obj, typename Ret>
		struct PyMeth<Ret(Obj::*)()>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <Ret(Obj::*NativeImpl)()>
			static PyObject* Meth(Type<Obj>* self)
			{
				Ret ret = (self->Get().*NativeImpl)();
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename Ret>
		struct PyMeth<Ret(Obj::*)() const>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <Ret(Obj::*NativeImpl)() const>
			static PyObject* Meth(Type<Obj>* self)
			{
				Ret ret = (self->Get().*NativeImpl)();
				return PyReturn(ret);
			}
		};

		template <typename Obj>
		struct PyMeth<void(Obj::*)()>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <void(Obj::*NativeImpl)()>
			static PyObject* Meth(Type<Obj>* self)
			{
				(self->Get().*NativeImpl)();
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj>
		struct PyMeth<void(Obj::*)() const>
		{
			constexpr static uint32_t FLAG = METH_NOARGS;
			template <void(Obj::*NativeImpl)() const>
			static PyObject* Meth(Type<Obj>* self)
			{
				(self->Get().*NativeImpl)();
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj, typename Ret>
		struct PyMeth<Ret(Obj::*)(PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <Ret(Obj::*NativeImpl)(PyKeywordParameters)>
			static PyObject* Meth(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				Ret ret = (self->Get().*NativeImpl)(PyKeywordParameters(kwds));
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename Ret>
		struct PyMeth<Ret(Obj::*)(PyKeywordParameters) const>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <Ret(Obj::*NativeImpl)(PyKeywordParameters) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				Ret ret = (self->Get().*NativeImpl)(PyKeywordParameters(kwds));
				return PyReturn(ret);
			}
		};

		template <typename Obj>
		struct PyMeth<void(Obj::*)(PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <void(Obj::*NativeImpl)(PyKeywordParameters)>
			static PyObject* Meth(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				(self->Get().*NativeImpl)(PyKeywordParameters(kwds));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj>
		struct PyMeth<void(Obj::*)(PyKeywordParameters) const>
		{
			constexpr static uint32_t FLAG = METH_NOARGS | METH_KEYWORDS;
			template <void(Obj::*NativeImpl)(PyKeywordParameters) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				(self->Get().*NativeImpl)(PyKeywordParameters(kwds));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		////////////           ////////////
		//////////// With args ////////////
		////////////           ////////////
		template <typename Obj, typename Ret, typename... Args>
		struct PyMeth<Ret(Obj::*)(Args...)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <Ret(Obj::*NativeImpl)(Args...)>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename Ret, typename... Args>
		struct PyMeth<Ret(Obj::*)(Args...) const>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <Ret(Obj::*NativeImpl)(Args...) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename... Args>
		struct PyMeth<void(Obj::*)(Args...)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(Obj::*NativeImpl)(Args...)>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj, typename... Args>
		struct PyMeth<void(Obj::*)(Args...) const>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(Obj::*NativeImpl)(Args...) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj, typename Ret, typename... Args>
		struct PyMeth<Ret(Obj::*)(PyKeywordParameters, Args...)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL | METH_KEYWORDS;
			template <Ret(Obj::*NativeImpl)(PyKeywordParameters, Args...)>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename Ret, typename... Args>
		struct PyMeth<Ret(Obj::*)(PyKeywordParameters, Args...) const>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL | METH_KEYWORDS;
			template <Ret(Obj::*NativeImpl)(PyKeywordParameters, Args...) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				Ret ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename... Args>
		struct PyMeth<void(Obj::*)(Args..., PyKeywordParameters)>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(Obj::*NativeImpl)(Args..., PyKeywordParameters)>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		template <typename Obj, typename... Args>
		struct PyMeth<void(Obj::*)(PyKeywordParameters, Args...) const>
		{
			constexpr static uint32_t FLAG = METH_FASTCALL;
			template <void(Obj::*NativeImpl)(PyKeywordParameters, Args...) const>
			static PyObject* Meth(Type<Obj>* self, PyObject* const* args, Py_ssize_t nargs, PyObject* kwds)
			{
				if (nargs != sizeof...(Args))
				{
					PyErr_BadArgument();
					return nullptr;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseArgs(args, arguments, std::make_integer_sequence<uint32_t, sizeof...(Args)>()))
					return nullptr;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				Py_INCREF(Py_None);
				return Py_None;
			}
		};

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Python __init__.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename MemFn>
		struct PyInit;

		template <typename Obj, typename... Args>
		struct PyInit<void(Obj::*)(Args...)>
		{
			template <void(Obj::*NativeImpl)(Args...)>
			static int Init(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseTuple(args, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), &arguments))
					return -1;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				return 0;
			}
		};

		// Partical specialization - The argument list #3: If any argument is a pack expansion, it must be the last argument in the list. 
		template <typename Obj, typename... Args>
		struct PyInit<void(Obj::*)(PyKeywordParameters, Args...)>
		{
			template <void(Obj::*NativeImpl)(PyKeywordParameters, Args...)>
			static int Init(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseTuple(args, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), &arguments))
					return -1;
				std::apply([&](Args... args) { (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				return 0;
			}
		};

		template <typename Obj, typename... Args>
		struct PyInit<PyRetVal<void>(Obj::*)(Args...)>
		{
			template <PyRetVal<void>(Obj::*NativeImpl)(Args...)>
			static int Init(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> arguments;
				if (!PyParseTuple(args, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), &arguments))
					return -1;
				PyRetVal<void> ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(std::move(args)...); }, std::move(arguments));
				if (ret.status == PyStatus::Success)
					return 0;
				if (ret.status == PyStatus::RaiseException)
					PyErr_SetNone(PyExc_RuntimeError);
				return -1;
			}
		};

		template <typename Obj, typename... Args>
		struct PyInit<PyRetVal<void>(Obj::*)(PyKeywordParameters, Args...)>
		{
			template <PyRetVal<void>(Obj::*NativeImpl)(PyKeywordParameters, Args...)>
			static int Init(Type<Obj>* self, PyObject* args, PyObject* kwds)
			{
				std::tuple<Args...> arguments;
				if (!PyParseTuple(args, std::make_integer_sequence<uint32_t, sizeof...(Args)>(), &arguments))
					return -1;
				PyRetVal<void> ret = std::apply([&](Args... args) { return (self->Get().*NativeImpl)(PyKeywordParameters(kwds), std::move(args)...); }, std::move(arguments));
				if (ret.status == PyStatus::Success)
					return 0;
				if (ret.status == PyStatus::RaiseException)
					PyErr_SetNone(PyExc_RuntimeError);
				return -1;
			}
		};

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Python __del__.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename MemFn>
		struct PyDel {};

		template <typename Obj>
		struct PyDel<void(Obj::*)()>
		{
			template <void(Obj::*NativeImpl)()>
			static void Del(Type<Obj>* self)
			{
				(self->Get().*NativeImpl)();
			}
		};

		/*————————————————————————————————————————————————————————————————————————————————————————————————————
				Python property.
		————————————————————————————————————————————————————————————————————————————————————————————————————*/
		template <typename MemFn>
		struct PyGetter {};

		template <typename Obj, typename Ret>
		struct PyGetter<Ret(Obj::*)()>
		{
			template <Ret(Obj::*NativeImpl)()>
			static PyObject* Get(Type<Obj>* self, void* userdata)
			{
				Ret ret = (self->Get().*NativeImpl)();
				return PyReturn(ret);
			}
		};

		template <typename Obj, typename Ret>
		struct PyGetter<Ret(Obj::*)() const>
		{
			template <Ret(Obj::*NativeImpl)() const>
			static PyObject* Get(Type<Obj>* self, void* userdata)
			{
				Ret ret = (self->Get().*NativeImpl)();
				return PyReturn(ret);
			}
		};

		template <typename MemFn>
		struct PySetter {};

		template <typename Obj, typename Arg>
		struct PySetter<void(Obj::*)(Arg arg)>
		{
			template <void(Obj::*NativeImpl)(Arg arg)>
			static int Set(Type<Obj>* self, PyObject* arg, void* userdata)
			{
				Arg parm;
				if (!PyParse(arg, &parm))
					return -1;
				(self->Get().*NativeImpl)(std::move(parm));
				return 0;
			}
		};

		template <typename Obj, typename Arg>
		struct PySetter<PyRetVal<void>(Obj::*)(Arg arg)>
		{
			template <PyRetVal<void>(Obj::*NativeImpl)(Arg arg)>
			static int Set(Type<Obj>* self, PyObject* arg, void* userdata)
			{
				Arg parm;
				if (!PyParse(arg, &parm))
					return -1;
				PyRetVal<void> ret = (self->Get().*NativeImpl)(std::move(parm));
				if (ret.status == PyStatus::Success)
					return 0;
				if (ret.status == PyStatus::RaiseException)
					PyErr_SetNone(PyExc_RuntimeError);
				return -1;
			}
		};
	}
}