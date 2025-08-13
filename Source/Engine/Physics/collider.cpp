#include "Engine/Physics/collider.h"
#include "Engine/Physics/physics.h"

using namespace glex::px;

bool Collider::Create(PhysicalMaterial material, Shape const& shape, glm::vec3 const& pos, glm::quat const& rot)
{
	m_shape = Physics::SDK().createShape(*shape.GetHandle(), *material.GetHandle());
	if (m_shape != nullptr)
	{
		m_shape->setLocalPose(physx::PxTransform(reinterpret_cast<physx::PxVec3 const&>(pos), reinterpret_cast<physx::PxQuat const&>(rot)));
		return true;
	}
	return false;
}

void Collider::Destroy()
{
	if (m_shape != nullptr)
		m_shape->release();
}