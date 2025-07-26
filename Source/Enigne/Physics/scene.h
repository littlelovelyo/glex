#pragma once
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>

namespace glex::px
{
	class Actor;
	class PhysicalScene
	{
	private:
		physx::PxScene* m_scene;

	public:
		PhysicalScene() : m_scene(nullptr) {}
		PhysicalScene(physx::PxScene* scene) : m_scene(scene) { }
		bool Create(glm::vec3 const& gravity);
		void Destroy();
		bool operator==(nullptr_t rhs) const { return m_scene == nullptr; }
		physx::PxScene* GetHandle() const { return m_scene; }
		void SetGravity(glm::vec3 const& gravity);
		void Add(Actor& actor);
		void Remove(Actor& actor);
		void Step(float dt);
	};
}