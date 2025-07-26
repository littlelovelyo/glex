#include "Engine/Scripting/module.h"

using namespace glex;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		MODULE
————————————————————————————————————————————————————————————————————————————————————————————————————*/
PyModule::PyModule(char const* name)
{
	m_module = PyImport_ImportModule(name);
	if (m_module == nullptr)
	{
#if GLEX_REPORT_SCRIPT_ERRORS
		PyErr_Print();
#else
		PyErr_Clear();
#endif
	}
}

PyModule::~PyModule()
{
	Py_XDECREF(m_module);
}

PyObject* PyModule::GetObject(char const* name)
{
	PyObject* o = PyObject_GetAttrString(m_module, name);
	if (o == nullptr)
	{
#if GLEX_REPORT_SCRIPT_ERRORS
		PyErr_Print();
#else
		PyErr_Clear();
#endif
	}
	return o;
}