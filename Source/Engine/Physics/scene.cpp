#include "Engine/Physics/scene.h"
#include "Engine/Physics/physics.h"
#include "Engine/Physics/actor.h"

using namespace glex::px;

bool PhysicalScene::Create(glm::vec3 const& gravity)
{
	physx::PxSceneDesc desc(Physics::SDK().getTolerancesScale());
	desc.cpuDispatcher = Physics::Dispatcher();
	desc.gravity = reinterpret_cast<physx::PxVec3 const&>(gravity);
	desc.filterShader = physx::PxDefaultSimulationFilterShader;
	m_scene = Physics::SDK().createScene(desc);
	return m_scene == nullptr;
}

void PhysicalScene::Destroy()
{
	if (m_scene != nullptr)
		m_scene->release();
}

void PhysicalScene::SetGravity(glm::vec3 const& gravity)
{
	m_scene->setGravity(reinterpret_cast<physx::PxVec3 const&>(gravity));
}

void PhysicalScene::Step(float dt)
{
	m_scene->simulate(dt / 1000.0f);
	m_scene->fetchResults(true);
}

void PhysicalScene::Add(Actor& actor)
{
	m_scene->addActor(*actor.GetHandle());
}

void PhysicalScene::Remove(Actor& actor)
{
	m_scene->removeActor(*actor.GetHandle());
}