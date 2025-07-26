#pragma once
#define PY_SSIZE_T_CLEAN
#include <Py/Python.h>

namespace glex::py
{
	PyObject* PyList_Pop(PyObject* list);
}