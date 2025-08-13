#pragma once
#include "Engine/Physics/physmat.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glex::px
{
	class Shape
	{
	protected:
		Shape() = default;

	public:
		physx::PxGeometry const* GetHandle() const { return reinterpret_cast<physx::PxGeometry const*>(this); }
	};

	class Sphere final : public Shape
	{
	private:
		physx::PxSphereGeometry m_sphere;

	public:
		Sphere(float radius) : m_sphere(radius) {}
	};

	class Box final : public Shape
	{
	private:
		physx::PxBoxGeometry m_box;

	public:
		Box(float hx, float hy, float hz) : m_box(hx, hy, hz) {}
	};

	class Capsule final : public Shape
	{
	private:
		physx::PxCapsuleGeometry m_capsule;

	public:
		Capsule(float radius, float halfHeight) : m_capsule(radius, halfHeight) {}
	};

	class Plane final : public Shape
	{
	private:
		physx::PxPlaneGeometry m_plane;

	public:
		Plane() {}
	};

	class Collider
	{
	private:
		physx::PxShape* m_shape;

	public:
		bool Create(PhysicalMaterial material, Shape const& shape, glm::vec3 const& pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::quat const& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
		void Destroy();
		bool operator==(nullptr_t rhs) const { return m_shape; }
		physx::PxShape* GetHandle() const { return m_shape; }
	};	
}