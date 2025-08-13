#include "Engine/Physics/rigid.h"
#include "Engine/Physics/physics.h"
#include "Core/assert.h"

using namespace glex::px;

void RigidActor::SetPose(glm::vec3 const& pos, glm::quat const& rot)
{
	m_rigidActor->setGlobalPose(physx::PxTransform(reinterpret_cast<physx::PxVec3 const&>(pos), reinterpret_cast<physx::PxQuat const&>(rot)));
}

bool StaticRigidActor::Create(glm::vec3 const& position, glm::quat const& rotation, Collider collider)
{
	m_staticRigidActor = Physics::SDK().createRigidStatic(physx::PxTransform(reinterpret_cast<physx::PxVec3 const&>(position), reinterpret_cast<physx::PxQuat const&>(rotation)));
	if (m_staticRigidActor != nullptr)
	{
		m_staticRigidActor->attachShape(*collider.GetHandle());
		return true;
	}
	return false;
}

bool DynamicRigidActor::Create(glm::vec3 const& position, glm::quat const& rotation, Collider collider)
{
	m_dynamicRigidActor = Physics::SDK().createRigidDynamic(physx::PxTransform(reinterpret_cast<physx::PxVec3 const&>(position), reinterpret_cast<physx::PxQuat const&>(rotation)));
	if (m_dynamicRigidActor != nullptr)
	{
		m_dynamicRigidActor->attachShape(*collider.GetHandle());
		return true;
	}
	return false;
}

void DynamicRigidActor::SetDensity(float density)
{
	GLEX_DEBUG_ASSERT(density > 0.0f) {}
	if (isinf(density))
		density = 0.0f;
	bool ret = physx::PxRigidBodyExt::updateMassAndInertia(*m_dynamicRigidActor, density);
	GLEX_DEBUG_ASSERT(ret) {};
}

float DynamicRigidActor::GetMass() const
{
	return m_dynamicRigidActor->getMass();
}

void DynamicRigidActor::SetMass(float mass)
{
	GLEX_DEBUG_ASSERT(mass > 0.0f) {}
	if (isinf(mass))
		mass = 0.0f;
	bool ret = physx::PxRigidBodyExt::setMassAndUpdateInertia(*m_dynamicRigidActor, mass);
	GLEX_DEBUG_ASSERT(ret) {};
}

void DynamicRigidActor::SetVelocity(glm::vec3 const& velocity)
{
	m_dynamicRigidActor->setLinearVelocity(reinterpret_cast<physx::PxVec3 const&>(velocity));
}

glm::vec3 DynamicRigidActor::GetVelocity() const
{
	auto result = m_dynamicRigidActor->getLinearVelocity();
	return reinterpret_cast<glm::vec3&>(result);
}

bool DynamicRigidActor::NeedsUpdate() const
{
	return !m_dynamicRigidActor->isSleeping();
}

std::pair<glm::vec3, glm::quat> DynamicRigidActor::GetTransform() const
{
	auto [rot, pos] = m_dynamicRigidActor->getGlobalPose();
	return std::make_pair(reinterpret_cast<glm::vec3&>(pos), reinterpret_cast<glm::quat&>(rot));
}