#pragma once
#include "Intermediate/Component/scene.h"
#include "Intermediate/Scripting/type.h"

namespace glex::py
{
	struct Scene
	{
		glex::Scene* p;
	};

	struct GameObject
	{
		Type<Scene>* scene;
		uint32_t entity;
	};

	struct Transform
	{
		glex::Transform* p;
	};

	PyObject* Scene_GetClearColor(Type<Scene>* self);
	PyObject* Scene_SetClearColor(Type<Scene>* self, PyObject* const* args, Py_ssize_t nargs);
	PyObject* Object_GetScene(Type<GameObject>* self);
	PyObject* Object_GetTransform(Type<GameObject>* self);
	PyObject* Transform_GetPosition(Type<Transform>* self);
	PyObject* Transform_GetRotation(Type<Transform>* self);
	PyObject* Transform_GetScale(Type<Transform>* self);
	int Transform_SetPosition(Type<Transform>* self, PyObject* args);
	int Transform_SetRotation(Type<Transform>* self, PyObject* args);
	int Transform_SetScale(Type<Transform>* self, PyObject* args);
	PyObject* Transform_Move(Type<Transform>* self, PyObject* const* args, Py_ssize_t nargs);
}