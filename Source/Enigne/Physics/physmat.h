#pragma once
#include "Core/commdefs.h"
#include <PxPhysicsAPI.h>

namespace glex::px
{
	class PhysicalMaterial
	{
	private:
		physx::PxMaterial* m_material;

	public:
		bool Create(float staticFriction, float dynamicFriction, float restitution);
		void Destroy();
		bool operator==(nullptr_t rhs) const { return m_material == nullptr; }
		physx::PxMaterial* GetHandle() const { return m_material; }
	};
}