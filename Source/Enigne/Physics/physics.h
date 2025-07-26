/**
 * Physics system implemented using PhysX.
 * Because of the complexity of the API,
 * we wrap it a little bit.
 */
#pragma once
#include "Engine/Physics/basic.h"
#include "Engine/Physics/scene.h"
#include "Engine/Physics/physmat.h"
#include "Core/Container/optional.h"
#include "Core/Container/dummy.h"
#include "Core/Container/basic.h"
#include <glm/glm.hpp>

namespace glex
{
	class Physics : private StaticClass
	{
	private:
		inline static px::Allocator s_allocator;
		inline static px::ErrorReporter s_errorReporter;
		inline static px::AsyncDispatcher s_asyncDispatcher;
		inline static physx::PxFoundation* s_foundation;
		inline static physx::PxPhysics* s_physics;
		// static Deque<Transform*, Allocator> s_updateList;

	public:
		// static void Update(Scene& scene);

#ifdef GLEX_INTERNAL
		static void Startup();
		static void Shutdown();
		static physx::PxPhysics& SDK() { return *s_physics; }
		static physx::PxCpuDispatcher* Dispatcher() { return &s_asyncDispatcher; }
#endif
	};
}