#include "rigidbody.h"
#include "scene.h"

using namespace glex;

RigidBody::RigidBody(Scene& scene, Transform const& transform, ResPtr<Collider> collider, bool isStatic) : m_static(isStatic)
{
	if (isStatic)
		m_actor = impl::StaticRigidActor(transform.GetGlobalPosition(), transform.GetGlobalRotation(), collider);
	else
		m_actor = impl::DynamicRigidActor(transform.GetGlobalPosition(), transform.GetGlobalRotation(), collider);
	scene.PhysicalScene().Add(m_actor);
	transform.ComfirmPhysicsChange();
}

RigidBody::~RigidBody()
{
	if (m_actor != nullptr)
		m_actor.GetScene().Remove(m_actor);
}