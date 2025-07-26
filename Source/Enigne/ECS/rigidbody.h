#pragma once
#include "Engine/Physics/rigid.h"
#include "Engine/Component/transform.h"
#include "Core/Memory/shared_ptr.h"

namespace glex
{
	class Scene;
	class RigidBody
	{
	private:
		union
		{
			px::RigidActor m_actor;
			px::StaticRigidActor m_staticActor;
			px::DynamicRigidActor m_dynamicActor;
		};
		bool m_static;

	public:
		RigidBody(Scene& scene, Transform const& transform, px::Collider collider, bool isStatic);
		~RigidBody();

		RigidBody(RigidBody&& rhs) : m_actor(std::move(rhs.m_actor)), m_static(rhs.m_static)
		{
			rhs.m_actor = px::RigidActor();
		}

		RigidBody& operator=(RigidBody&& rhs)
		{
			std::swap(m_actor, rhs.m_actor);
			std::swap(m_static, rhs.m_static);
			return *this;
		}

#ifdef GLEX_INTERNAL
		bool NeedsUpdate() const
		{
			return !m_static && m_dynamicActor.NeedsUpdate();
		}

		std::pair<glm::vec3, glm::quat> GetPose() const
		{
			return m_dynamicActor.GetTransform();
		}

		void SetPose(glm::vec3 const& pos, glm::quat const& rot)
		{
			m_actor.SetPose(pos, rot);
		}
#endif
	};
}