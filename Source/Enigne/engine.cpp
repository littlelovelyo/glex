#include "Engine/engine.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Physics/physics.h"
#include "Engine/resource.h"
#include "Core/Thread/task.h"
#include "game.h"
#include <Windows.h>

using namespace glex;

void Engine::Startup(EngineStartupInfo const& appInfo)
{
	// Make sure this is bind before anyone else and we're good to go.
	Logger::Info("Current directory: %s", Platform::GetWorkingDirectory().Get());
	Window::GetSizeDelegate().Bind(Engine::OnResize);
	Window::Startup(appInfo.window);
	Scripting::Startup(appInfo.script);
	Renderer::Startup(appInfo.render);
	Async::Startup(appInfo.numWorkingThreads);
	Physics::Startup();
}

void Engine::Shutdown()
{
	Physics::Shutdown();
	Async::Shutdown();
	Renderer::Shutdown();
	Scripting::Shutdown();
	Window::Shutdown();
	Event::Clear();
#if GLEX_REPORT_MEMORY_LEAKS
	ResourceManager::FreeMemory();
#endif
}

void Engine::Tick()
{
	if (!Window::IsMinimized())
		Renderer::Tick();
}

void Engine::OnResize(uint32_t width, uint32_t height)
{
	Renderer::Resize();
	GameInstance::BaseResize(width, height);
	GameInstance::Get().Resize(width, height);
}