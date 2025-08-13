#pragma once
#include "Engine/Physics/scene.h"
#include <glm/gtc/quaternion.hpp>

namespace glex::px
{
	class Actor
	{
	protected:
		union
		{
			physx::PxActor* m_actor;
			physx::PxRigidActor* m_rigidActor;
			physx::PxRigidStatic* m_staticRigidActor;
			physx::PxRigidDynamic* m_dynamicRigidActor;
		};

	public:
		Actor() : m_actor(nullptr) {}
		void Destroy();
		bool operator==(nullptr_t rhs) const { return m_actor == nullptr; }
		physx::PxActor* GetHandle() { return m_actor; }
		PhysicalScene GetScene() const;
	};
}