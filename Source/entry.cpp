#include "game.h"
#include "Engine/engine.h"
#include <Windows.h>

using namespace glex;

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

inline static void EntryPoint()
{
	GameInstance& gameInstance = GameInstance::Get();
	EngineStartupInfo startupInfo;
	gameInstance.Preinitialize(startupInfo);
	Engine::Startup(startupInfo);
	gameInstance.BeginPlay();
	while (!Window::IsClosing())
	{
		Window::HandleEvents();
		gameInstance.Tick();
		Engine::Tick();
	}
	gameInstance.EndPlay();
	gameInstance.Shutdown();
	Engine::Shutdown();
#if GLEX_REPORT_MEMORY_LEAKS
	Mem::Report();
#endif
}

int main()
{
	EntryPoint();
	return 0;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	EntryPoint();
	return 0;
}