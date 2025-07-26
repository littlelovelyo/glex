#pragma once
#define PY_SSIZE_T_CLEAN
#include <Py/Python.h>

namespace glex::py
{
	PyObject* Information(PyObject* self);
	PyObject* Version(PyObject* self);
	PyObject* CurrentTime(PyObject* self);
	PyObject* DeltaTime(PyObject* self);
	PyObject* MouseX(PyObject* self);
	PyObject* MouseY(PyObject* self);
	PyObject* MouseDeltaX(PyObject* self);
	PyObject* MouseDeltaY(PyObject* self);
	PyObject* MouseScroll(PyObject* self);
	PyObject* SetTitle(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* Pressed(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* Pressing(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* Width(PyObject* self);
	PyObject* Height(PyObject* self);
	PyObject* Size(PyObject* self);
	PyObject* SetWidth(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* SetHeight(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* SetSize(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* Close(PyObject* self);
	PyObject* SetVSync(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* CaptureMouse(PyObject* self);
	PyObject* FreeMouse(PyObject* self);
	PyObject* SetFullScreen(PyObject* self);
	PyObject* SetWindowed(PyObject* self, PyObject* const* args, Py_ssize_t nargs);
}