#include "Engine/Scripting/helper.h"
#include "Engine/Scripting/template.h"

using namespace glex::py;

PyObject* glex::py::PyList_Pop(PyObject* list)
{
	PyObject* name = PyReturn("pop");
	PyObject* result = PyObject_CallMethodObjArgs(list, name, nullptr);
	Py_DECREF(name);
	return result;
}