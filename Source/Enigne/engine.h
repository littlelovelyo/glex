#pragma once
#include "Core/Platform/window.h"
#include "Core/GL/context.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Scripting/scripting.h"

namespace glex
{
	struct EngineStartupInfo
	{
		WindowStartupInfo window;
		ScriptStartupInfo script;
		RendererStartupInfo render;
		uint32_t numWorkingThreads = 0;
	};

	class Engine : private StaticClass
	{
	public:
		static void Startup(EngineStartupInfo const& info);
		static void Shutdown();
		static void Tick();
		static void OnResize(uint32_t width, uint32_t height);
	};
}