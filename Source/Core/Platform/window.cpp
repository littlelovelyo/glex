#include "Core/Platform/window.h"
#include "Core/assert.h"
#include "Core/Platform/input.h"
#include "Core/Platform/time.h"
#include "Core/Platform/platform.h"
#include "Core/GL/context.h"
#include "Core/log.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace glex;

// Hidden static members.
static GLFWmonitor* s_monitor;
static GLFWwindow* s_window;
static HWND s_hWnd;
static WNDPROC s_glfwWndProc;
static bool s_windowResizeDragging;
static bool s_resizeDelayed;

static LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_ENTERSIZEMOVE:
		{
			s_windowResizeDragging = true;
			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			s_windowResizeDragging = false;
			if (s_resizeDelayed)
			{
				Window::GetSizeDelegate().Broadcast(Window::Width(), Window::Height());
				s_resizeDelayed = false;
			}
			return 0;
		}
		default: return CallWindowProcW(s_glfwWndProc, hWnd, uMsg, wParam, lParam);
	}
}

void Window::Startup(WindowStartupInfo const& info)
{
	GLEX_DEBUG_ASSERT(info.minHeight > 0) {}
	GLFWallocator allocator;
	allocator.allocate = [](size_t size, void* user) { return Mem::Alloc(size); };
	allocator.reallocate = [](void* block, size_t size, void* user) { return Mem::Realloc(block, size); };
	allocator.deallocate = [](void* block, void* user) { Mem::Free(block); };
	allocator.user = nullptr;
	glfwInitAllocator(&allocator);
	if (!glfwInit())
		Logger::Fatal("Cannot init GLFW.");
#if GLEX_REPORT_GLFW_ERRORS
	glfwSetErrorCallback([](int error, char const* desc)
	{
		Logger::Error("GLFW error: %s", desc);
	});
#endif
	if (info.fullscreenMonitor == -1)
		s_monitor = nullptr;
	else if (info.fullscreenMonitor == 0)
		s_monitor = glfwGetPrimaryMonitor();
	else
	{
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		if (info.fullscreenMonitor >= count)
		{
			Logger::Error("Invalid monitor. Use primary monitor instead.");
			s_monitor = glfwGetPrimaryMonitor();
		}
		else
			s_monitor = monitors[info.fullscreenMonitor];
	}

	glfwWindowHint(GLFW_RESIZABLE, info.resizable);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// Don't create OpenGL context.
	s_window = glfwCreateWindow(info.width, info.height, info.title, info.fullScreen ? s_monitor : nullptr, nullptr);
	if (s_window == nullptr)
		Logger::Fatal("Cannot create window.");
	glfwSetWindowSizeLimits(s_window, info.minWidth, info.minHeight, info.maxWidth, info.maxHeight);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(s_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwGetFramebufferSize(s_window, reinterpret_cast<int*>(&s_width), reinterpret_cast<int*>(&s_height));

	/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
			Window size hack.
			May change GLFW's source code but it will probably break something.
	 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
	s_hWnd = glfwGetWin32Window(s_window);
	s_glfwWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(s_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));
	if (s_glfwWndProc == nullptr)
		Logger::Fatal("Cannot hack window procedure.");

	/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
			Callbacks.
	 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
	glfwSetFramebufferSizeCallback(s_window, [](GLFWwindow* window, int width, int height)
	{
		if (width == 0)
			s_minimized = true;
		else
		{
			if (s_minimized)
			{
				s_minimized = false;
				if (s_width == width && s_height == height)
					return;
			}
			s_width = width;
			s_height = height;
			if (!s_windowResizeDragging)
				s_sizeDelegate.Broadcast(width, height);
			else
				s_resizeDelayed = true;
		}
	});
	glfwSetKeyCallback(s_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		// 44~96.
		static uint8_t s_keyTable44[]
		{
			KeyCode::Comma,
			KeyCode::Minus,
			KeyCode::Period,
			KeyCode::Slash,
			KeyCode::Key0,
			KeyCode::Key1,
			KeyCode::Key2,
			KeyCode::Key3,
			KeyCode::Key4,
			KeyCode::Key5,
			KeyCode::Key6,
			KeyCode::Key7,
			KeyCode::Key8,
			KeyCode::Key9,
			0,
			KeyCode::Semicolon,
			0,
			KeyCode::Equal,
			0,
			0,
			0,
			KeyCode::A,
			KeyCode::B,
			KeyCode::C,
			KeyCode::D,
			KeyCode::E,
			KeyCode::F,
			KeyCode::G,
			KeyCode::H,
			KeyCode::I,
			KeyCode::J,
			KeyCode::K,
			KeyCode::L,
			KeyCode::M,
			KeyCode::N,
			KeyCode::O,
			KeyCode::P,
			KeyCode::Q,
			KeyCode::R,
			KeyCode::S,
			KeyCode::T,
			KeyCode::U,
			KeyCode::V,
			KeyCode::W,
			KeyCode::X,
			KeyCode::Y,
			KeyCode::Z,
			KeyCode::LeftBracket,
			KeyCode::Backslash,
			KeyCode::RightBracket,
			0,
			0,
			KeyCode::GraveAccent
		};
		// 256~348
		static uint8_t s_keyTable256[] =
		{
			KeyCode::Escape,
			KeyCode::Return,
			KeyCode::Tab,
			KeyCode::Backspace,
			KeyCode::Insert,
			KeyCode::Delete,
			KeyCode::Right,
			KeyCode::Left,
			KeyCode::Down,
			KeyCode::Up,
			KeyCode::PageUp,
			KeyCode::PageDown,
			KeyCode::Home,
			KeyCode::End,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			KeyCode::CapsLock,
			KeyCode::ScrollLock,
			KeyCode::NumLock,
			KeyCode::PrintScreen,
			0,
			0, 0, 0, 0, 0,
			KeyCode::F1,
			KeyCode::F2,
			KeyCode::F3,
			KeyCode::F4,
			KeyCode::F5,
			KeyCode::F6,
			KeyCode::F7,
			KeyCode::F8,
			KeyCode::F9,
			KeyCode::F10,
			KeyCode::F11,
			KeyCode::F12,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			KeyCode::Num0,
			KeyCode::Num1,
			KeyCode::Num2,
			KeyCode::Num3,
			KeyCode::Num4,
			KeyCode::Num5,
			KeyCode::Num6,
			KeyCode::Num7,
			KeyCode::Num8,
			KeyCode::Num9,
			KeyCode::NumDecimal,
			KeyCode::NumDivide,
			KeyCode::NumMultiply,
			KeyCode::NumSubtract,
			KeyCode::NumAdd,
			KeyCode::NumReturn,
			0, 0, 0, 0,
			KeyCode::LeftShift,
			KeyCode::LeftControl,
			KeyCode::LeftAlt,
			KeyCode::LeftWindows,
			KeyCode::RightShift,
			KeyCode::RightControl,
			KeyCode::RightAlt,
			KeyCode::RightWindows,
			KeyCode::Menu
		};

		uint8_t virtualCode = 0;
		if (key < 97)
		{
			if (key < 44)
			{
				if (key == 32)
					virtualCode = KeyCode::Space;
				else if (key == 39)
					virtualCode = KeyCode::Apostrophe;
			}
			else
				virtualCode = s_keyTable44[key - 44];
		}
		else if (key > 255 && key < 349)
			virtualCode = s_keyTable256[key - 256];
		if (virtualCode == 0)
			return;

		if (action == GLFW_PRESS)
			Input::Press(virtualCode);
		else if (action == GLFW_RELEASE)
			Input::Release(virtualCode);
	});
	glfwSetCharCallback(s_window, [](GLFWwindow* window, uint32_t code)
	{
		Input::InputChar(code);
	});
	glfwSetMouseButtonCallback(s_window, [](GLFWwindow* window, int button, int action, int mods)
	{
		switch (button)
		{
			case 0: button = KeyCode::LMB; break;
			case 1: button = KeyCode::RMB; break;
			case 2: button = KeyCode::MMB; break;
			default: return;
		}
		if (action == GLFW_PRESS)
			Input::Press(button);
		else
			Input::Release(button);
	});
	glfwSetCursorPosCallback(s_window, [](GLFWwindow* window, double xpos, double ypos)
	{
		Input::Move(xpos, ypos);
	});
	glfwSetScrollCallback(s_window, [](GLFWwindow* window, double xoffset, double yoffset)
	{
		Input::Scroll(yoffset);
	});

	Input::Startup();
	Time::Startup();
}

void Window::Shutdown()
{
	Input::Shutdown();
#if GLEX_REPORT_MEMORY_LEAKS
	s_sizeDelegate.Clear();
#endif
	glfwDestroyWindow(s_window);
	glfwTerminate();
}

uint64_t Window::GetWin32Handle()
{
	return reinterpret_cast<uint64_t>(s_hWnd);
}

void Window::HandleEvents()
{
	Input::Update();
	glfwPollEvents();
	Time::Update();
}

void Window::SetSize(uint32_t width, uint32_t height)
{
	glfwSetWindowSize(s_window, width, height);
}

void Window::SetSizeLimits(int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight)
{
	glfwSetWindowSizeLimits(s_window, minWidth, minHeight, maxWidth, maxHeight);
}

bool Window::IsClosing()
{
	return glfwWindowShouldClose(s_window);
}

void Window::Close()
{
	glfwSetWindowShouldClose(s_window, 1);
}

void Window::CaptureMouse()
{
	glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::FreeMouse()
{
	glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::SetCursor(Cursor cursor)
{
	if (cursor == s_currentCursor)
		return;
	static GLFWcursor* s_cursors[3];
	GLFWcursor* cursorPtr = nullptr;
	switch (cursor)
	{
		case Cursor::Arrow:
		{
			if (s_cursors[0] == nullptr)
				s_cursors[0] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
			cursorPtr = s_cursors[0];
			break;
		}
		case Cursor::HorizontalResize:
		{
			if (s_cursors[1] == nullptr)
				s_cursors[1] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
			cursorPtr = s_cursors[1];
			break;
		}
		case Cursor::VerticalResize:
		{
			if (s_cursors[2] == nullptr)
				s_cursors[2] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
			cursorPtr = s_cursors[2];
			break;
		}
	}
	if (cursorPtr != nullptr)
	{
		glfwSetCursor(s_window, cursorPtr);
		s_currentCursor = cursor;
	}
}

void Window::SetWindowed(int32_t xPos, int32_t yPos, int32_t width, int32_t height)
{
	glfwSetWindowMonitor(s_window, nullptr, xPos, yPos, width, height, 0);
}

void Window::SetTitle(char const* title)
{
	glfwSetWindowTitle(s_window, title);
}

void Window::SetFullScreen()
{
	GLFWvidmode const* mode = glfwGetVideoMode(s_monitor);
	glfwSetWindowMonitor(s_window, s_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

VkSurfaceKHR Window::CreateSurface(VkInstance instance)
{
	VkSurfaceKHR result;
	!glfwCreateWindowSurface(instance, s_window, gl::Context::HostAllocator(), &result);
	return result;
}