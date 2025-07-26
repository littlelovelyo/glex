#include "Engine/Physics/physics.h"
#include "Core/Platform/time.h"
#include "Core/assert.h"

using namespace glex;
using namespace glex::px;

void Physics::Startup()
{
	Logger::Trace("PhysX version: %d.%d.%d.", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);
	s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_allocator, s_errorReporter);
	GLEX_ASSERT_MSG(s_foundation != nullptr, "Cannot launch PhysX.") {}
	s_foundation->setReportAllocationNames(false);
	s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_foundation, physx::PxTolerancesScale());
	GLEX_ASSERT_MSG(s_physics != nullptr, "Cannot launch PhysX.") {}
}

void Physics::Shutdown()
{
	s_physics->release();
	s_foundation->release();
}

//void Physics::Update(Scene& scene)
//{
//	PhysicalScene& pxScene = scene.PhysicalScene();
//	pxScene.Step(Time::DeltaTime());
//	scene.ForEach<RigidBody, Transform>([](uint32_t id, RigidBody& rb, Transform& trans)
//	{
//		if (trans.PhysicsDirty())
//		{
//			rb.SetPose(trans.GetGlobalPosition(), trans.GetGlobalRotation());
//			trans.ComfirmPhysicsChange();
//			return;
//		}
//		if (!rb.NeedsUpdate())
//			return;
//		s_updateList.push_back(&trans);
//		auto [pos, rot] = rb.GetPose();
//		trans.LockAt(pos, rot);
//	});
//	for (Transform* trans : s_updateList)
//		trans->Unlock();
//	s_updateList.clear();
//}