#include "Intermediate/Scripting/fn.h"
#include "Core/Native/window.h"
#include "Core/Native/time.h"
#include "Core/Native/input.h"

namespace glex::py
{
	PyObject* Information(PyObject* self)
	{
		return PyUnicode_FromString("glex version 0.2, made with C++ and Vulkan.");
	}

	PyObject* Version(PyObject* self)
	{
		return PyLong_FromLong(0x20250116);
	}

	/*static PyObject* s_timeCache, *s_deltaTimeCache, *s_mouseX, *s_mouseY, *s_mouseDeltaX, *s_mouseDeltaY, *s_mouseScroll;

	void SetFrameCache(double time, double deltaTime)
	{
		Py_XDECREF(s_timeCache);
		Py_XDECREF(s_deltaTimeCache);
		Py_XDECREF(s_mouseX);
		Py_XDECREF(s_mouseY);
		Py_XDECREF(s_mouseDeltaX);
		Py_XDECREF(s_mouseDeltaY);
		Py_XDECREF(s_mouseScroll);
		s_timeCache = PyFloat_FromDouble(glex::Time::Current());
		s_deltaTimeCache = PyFloat_FromDouble(glex::Time::DeltaTime());
		s_mouseX = PyFloat_FromDouble(glex::Input::MouseX());
		s_mouseY = PyFloat_FromDouble(glex::Input::MouseY());
		s_mouseDeltaX = PyFloat_FromDouble(glex::Input::DeltaX());
		s_mouseDeltaY = PyFloat_FromDouble(glex::Input::DeltaY());
		s_mouseScroll = PyFloat_FromDouble(glex::Input::Scroll());
		Py_INCREF(s_timeCache);
		Py_INCREF(s_deltaTimeCache);
		Py_INCREF(s_mouseX);
		Py_INCREF(s_mouseY);
		Py_INCREF(s_mouseDeltaX);
		Py_INCREF(s_mouseDeltaY);
		Py_INCREF(s_mouseScroll);
	}

	void CleanCache()
	{
		Py_XDECREF(s_timeCache);
		Py_XDECREF(s_deltaTimeCache);
		Py_XDECREF(s_mouseDeltaX);
		Py_XDECREF(s_mouseDeltaY);
	}*/

	PyObject* CurrentTime(PyObject* self)
	{
		return s_timeCache;
	}

	PyObject* DeltaTime(PyObject* self)
	{
		return s_deltaTimeCache;
	}

	PyObject* MouseX(PyObject* self)
	{
		return s_mouseX;
	}

	PyObject* MouseY(PyObject* self)
	{
		return s_mouseY;
	}

	PyObject* MouseDeltaX(PyObject* self)
	{
		return s_mouseDeltaX;
	}

	PyObject* MouseDeltaY(PyObject* self)
	{
		return s_mouseDeltaY;
	}

	PyObject* MouseScroll(PyObject* self)
	{
		return s_mouseScroll;
	}

	PyObject* SetTitle(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		char const* title = PyUnicode_AsUTF8(args[0]);
		if (title == nullptr)
			return nullptr;
		glex::Window::SetTitle(title);
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* Pressed(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		uint32_t keyCode = PyLong_AsInt(args[0]);
		if (keyCode > glex::Input::KeyCount || !glex::Input::Pressed(static_cast<glex::Input::Key>(keyCode)))
		{
			Py_INCREF(Py_False);
			return Py_False;
		}
		Py_INCREF(Py_True);
		return Py_True;
	}

	PyObject* Pressing(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		uint32_t keyCode = PyLong_AsInt(args[0]);
		if (keyCode > glex::Input::KeyCount || !glex::Input::Pressing(static_cast<glex::Input::Key>(keyCode)))
		{
			Py_INCREF(Py_False);
			return Py_False;
		}
		Py_INCREF(Py_True);
		return Py_True;
	}

	PyObject* Width(PyObject* self)
	{
		return PyLong_FromLong(glex::Window::Width());
	}

	PyObject* Height(PyObject* self)
	{
		return PyLong_FromLong(glex::Window::Height());
	}

	PyObject* Size(PyObject* self)
	{
		PyObject* ret = PyTuple_New(2);
		PyTuple_SetItem(ret, 0, PyLong_FromLong(glex::Window::Width()));
		PyTuple_SetItem(ret, 1, PyLong_FromLong(glex::Window::Height()));
		return ret;
	}

	PyObject* SetWidth(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		uint32_t width = PyLong_AsInt(args[0]);
		glex::Window::SetSize(width, glex::Window::Height());
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* SetHeight(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		uint32_t height = PyLong_AsInt(args[0]);
		glex::Window::SetSize(glex::Window::Width(), height);
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* SetSize(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 2)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		uint32_t width = PyLong_AsInt(args[0]);
		uint32_t height = PyLong_AsInt(args[1]);
		glex::Window::SetSize(width, height);
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* Close(PyObject* self)
	{
		glex::Window::Close();
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* SetVSync(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 1)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		// glex::Renderer::SetVSync(PyObject_IsTrue(args[0]));
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* CaptureMouse(PyObject* self)
	{
		glex::Window::CaptureMouse();
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* FreeMouse(PyObject* self)
	{
		glex::Window::FreeMouse();
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* SetFullScreen(PyObject* self)
	{
		glex::Window::SetFullScreen();
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* SetWindowed(PyObject* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 4)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		glex::Window::SetWindowed(PyLong_AsLong(args[0]), PyLong_AsLong(args[1]), PyLong_AsLong(args[2]), PyLong_AsLong(args[3]));
		Py_INCREF(Py_None);
		return Py_None;
	}
}