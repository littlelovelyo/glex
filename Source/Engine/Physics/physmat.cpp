#include "Engine/Physics/physmat.h"
#include "Engine/Physics/physics.h"

using namespace glex::px;

bool PhysicalMaterial::Create(float staticFriction, float dynamicFriction, float restitution)
{
	m_material = Physics::SDK().createMaterial(staticFriction, dynamicFriction, restitution);
	return m_material != nullptr;
}

void PhysicalMaterial::Destroy()
{
	if (m_material != nullptr)
		m_material->release();
}