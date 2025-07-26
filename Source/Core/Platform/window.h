/**
 * The window.
 */
#pragma once
#include "Core/Container/delegate.h"
#include <vulkan/vulkan.h>

namespace glex
{
	struct WindowStartupInfo
	{
		int16_t width = 640, height = 480, fullscreenMonitor = -1;
		int16_t minWidth = 1, minHeight = 1;
		int16_t maxWidth = -1, maxHeight = -1;
		char const* title = "";
		bool resizable = true, fullScreen = false;
	};

	enum class Cursor : uint32_t
	{
		Arrow,
		HorizontalResize,
		VerticalResize
	};

	class Window : private StaticClass
	{
	private:
		inline static uint32_t s_width, s_height;
		inline static bool s_minimized;
		inline static Delegate<void(uint32_t, uint32_t)> s_sizeDelegate;
		inline static Cursor s_currentCursor = Cursor::Arrow;

	public:
		// fullScreenMonitor: -1 - windowed mode, 0 - primary monitor, 1... - other monitors
		static void Startup(WindowStartupInfo const& info);
		static void Shutdown();
		static uint64_t GetWin32Handle();
		// static GLFWwindow* GetHandle() { return s_window; }
		static void HandleEvents();
		static uint32_t Width() { return s_width; }
		static uint32_t Height() { return s_height; }
		static void SetSize(uint32_t width, uint32_t height);
		static void SetSizeLimits(int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight);
		static bool IsMinimized() { return s_minimized; }
		static bool IsClosing();
		static void Close();
		// static void SetVSync(bool enabled);
		static void CaptureMouse();
		static void FreeMouse();
		static void SetCursor(Cursor cursor);
		static void SetWindowed(int32_t xPos, int32_t yPos, int32_t width, int32_t height);
		static void SetTitle(char const* title);
		static void SetFullScreen();
		static auto& GetSizeDelegate() { return s_sizeDelegate; }
#if GLEX_INTERNAL
		static VkSurfaceKHR CreateSurface(VkInstance instance);
#endif
	};
}