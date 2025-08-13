#pragma once
#include "Engine/Physics/actor.h"
#include "Engine/Physics/collider.h"
#include <glm/glm.hpp>

namespace glex::px
{
	class RigidActor : public Actor
	{
	public:
		void SetPose(glm::vec3 const& pos, glm::quat const& rot);
	};

	class StaticRigidActor final : public RigidActor
	{
	public:
		bool Create(glm::vec3 const& position, glm::quat const& rotation, Collider collider);
	};

	class DynamicRigidActor final : public RigidActor
	{
	public:
		bool Create(glm::vec3 const& position, glm::quat const& rotation, Collider collider);
		void SetDensity(float density);
		void SetMass(float mass);
		float GetMass() const;
		void SetVelocity(glm::vec3 const& velocity);
		glm::vec3 GetVelocity() const;
		bool NeedsUpdate() const;
		std::pair<glm::vec3, glm::quat> GetTransform() const;
	};
}