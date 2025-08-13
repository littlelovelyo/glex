#include "Engine/Scripting/basic.h"

using namespace glex;

PyKeywordParameters::Iterator& PyKeywordParameters::Iterator::operator++()
{
	PyObject* key;
	if (PyDict_Next(m_dict, &m_pos, &key, &m_value))
		m_key = PyUnicode_AsUTF8(key);
	else
		m_pos = -1;
	return *this;
}