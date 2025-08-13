#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glex
{
	// Forward is positive Z;
	// Left is positive X.
	class Transform
	{
		static constexpr auto in_place_delete = true;

	private:
		Transform* m_parent;
		Transform* m_child, *m_prev, *m_next;
		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale;
		mutable glm::vec3 m_globalPosition;
		mutable glm::quat m_globalRotation;
		mutable glm::vec3 m_globalScale;
		mutable glm::mat4 m_modelMat;

		constexpr static uint32_t DIRTY_FLAG_GLOBAL = 1;
		constexpr static uint32_t DIRTY_FLAG_MATRIX = 2;
		constexpr static uint32_t DIRTY_FLAG_PHYSICS = 4;
		constexpr static uint32_t DIRTY_FLAG_ALL = DIRTY_FLAG_GLOBAL | DIRTY_FLAG_MATRIX | DIRTY_FLAG_PHYSICS;
		// If transform is modified during a physics update, we do not propagate it to its children.
		// If this flag is set, setting of the dirty flags is ignored.
		constexpr static uint32_t FLAG_LOCK = 8;
		mutable uint32_t m_flags;

		void SetDirtyFlag();
		void SetChildrenDirty();
		void FlushGlobal() const;

	public:
		Transform();
		Transform* GetParent() const { return m_parent; }
		Transform* GetChild() const { return m_child; }
		Transform* NextSibling() const { return m_next; }
		void SetParent(Transform* parent);
		void SetPosition(glm::vec3 const& pos);
		void SetRotation(glm::quat const& rot);
		void SetRotation(glm::vec3 const& euler);
		void SetScale(glm::vec3 const& scale);
		void SetGlobalPosition(glm::vec3 const& pos);
		void SetGlobalRotation(glm::quat const& rot);
		glm::vec3 const& Position() const { return m_position; }
		glm::quat const& Rotation() const { return m_rotation; }
		glm::vec3 const& Scale() const { return m_scale; }
		glm::vec3 const& GetGlobalPosition() const;
		glm::quat const& GetGlobalRotation() const;
		glm::vec3 const& GetGlobalScale() const;
		glm::mat4 const& GetModelMat() const;
		glm::vec3 Forward() const;
		glm::vec3 Left() const;
		void LookTowrards(glm::vec3 const& direction);
		void Move(glm::vec3 const& pos) { SetPosition(m_position + pos); }
		void Rotate(glm::quat const& rot) { SetRotation(rot * m_rotation); }
		void Rotate(glm::vec3 const& axis, float angle);

#ifdef GLEX_INTERNAL // Used by physics update.
		void LockAt(glm::vec3 const& pos, glm::quat const& rot);
		void Unlock();
		bool PhysicsDirty() const { return m_flags & DIRTY_FLAG_PHYSICS; }
		void ComfirmPhysicsChange() const { m_flags &= ~DIRTY_FLAG_PHYSICS; }
#endif
	};
}