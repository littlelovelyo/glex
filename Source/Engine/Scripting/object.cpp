#include "Intermediate/Scripting/object.h"

namespace glex::py
{
	PyObject* Scene_GetClearColor(Type<Scene>* self)
	{
		glm::vec3 const& color = (*self)->p->GetClearColor();
		PyObject* ret = PyTuple_New(3);
		PyTuple_SetItem(ret, 0, PyFloat_FromDouble(color.r));
		PyTuple_SetItem(ret, 1, PyFloat_FromDouble(color.g));
		PyTuple_SetItem(ret, 2, PyFloat_FromDouble(color.b));
		return ret;
	}

	PyObject* Scene_SetClearColor(Type<Scene>* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 3)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		glm::vec3 color;
		color.r = PyFloat_AsDouble(args[0]);
		color.g = PyFloat_AsDouble(args[1]);
		color.b = PyFloat_AsDouble(args[2]);
		(*self)->p->SetClearColor(color);
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* Object_GetScene(Type<GameObject>* self)
	{
		Py_INCREF((*self)->scene);
		return reinterpret_cast<PyObject*>((*self)->scene);
	}

	PyObject* Object_GetTransform(Type<GameObject>* self)
	{
		Type<Transform>* trans = Type<Transform>::New();
		(*trans)->p = &(*(*self)->scene)->p->GetComponent<glex::Transform>((*self)->entity);
		return reinterpret_cast<PyObject*>(trans);
	}

	PyObject* Transform_GetPosition(Type<Transform>* self)
	{
		glm::vec3 const& pos = (*self)->p->Position();
		PyObject* ret = PyTuple_New(3);
		PyTuple_SetItem(ret, 0, PyFloat_FromDouble(pos.x));
		PyTuple_SetItem(ret, 1, PyFloat_FromDouble(pos.y));
		PyTuple_SetItem(ret, 2, PyFloat_FromDouble(pos.z));
		return ret;
	}

	PyObject* Transform_GetRotation(Type<Transform>* self)
	{
		glm::quat const& rot = (*self)->p->Rotation();
		PyObject* ret = PyTuple_New(4);
		PyTuple_SetItem(ret, 0, PyFloat_FromDouble(rot.w));
		PyTuple_SetItem(ret, 1, PyFloat_FromDouble(rot.x));
		PyTuple_SetItem(ret, 2, PyFloat_FromDouble(rot.y));
		PyTuple_SetItem(ret, 3, PyFloat_FromDouble(rot.z));
		return ret;
	}

	PyObject* Transform_GetScale(Type<Transform>* self)
	{
		glm::vec3 const& scale = (*self)->p->Scale();
		PyObject* ret = PyTuple_New(3);
		PyTuple_SetItem(ret, 0, PyFloat_FromDouble(scale.x));
		PyTuple_SetItem(ret, 1, PyFloat_FromDouble(scale.y));
		PyTuple_SetItem(ret, 2, PyFloat_FromDouble(scale.z));
		return ret;
	}

	int Transform_SetPosition(Type<Transform>* self, PyObject* args)
	{
		glm::vec3 position;
		if (!PyArg_ParseTuple(args, "fff", &position[0], &position[1], &position[2]))
		{
			PyErr_BadArgument();
			return -1;
		}
		(*self)->p->SetPosition(position);
		return 0;
	}

	int Transform_SetRotation(Type<Transform>* self, PyObject* args)
	{
		glm::quat rotation;
		if (!PyArg_ParseTuple(args, "ffff", &rotation.w, &rotation.x, &rotation.y, &rotation.z))
		{
			PyErr_BadArgument();
			return -1;
		}
		(*self)->p->SetRotation(rotation);
		return 0;
	}

	int Transform_SetScale(Type<Transform>* self, PyObject* args)
	{
		glm::vec3 scale;
		if (!PyArg_ParseTuple(args, "fff", &scale[0], &scale[1], &scale[2]))
		{
			PyErr_BadArgument();
			return -1;
		}
		(*self)->p->SetScale(scale);
		return 0;
	}

	PyObject* Transform_Move(Type<Transform>* self, PyObject* const* args, Py_ssize_t nargs)
	{
		if (nargs != 3)
		{
			PyErr_BadArgument();
			return nullptr;
		}
		glm::vec3 move;
		move.x = PyFloat_AsDouble(args[0]);
		move.y = PyFloat_AsDouble(args[1]);
		move.z = PyFloat_AsDouble(args[2]);
		(*self)->p->Move(move);
		Py_INCREF(Py_None);
		return Py_None;
	}
}