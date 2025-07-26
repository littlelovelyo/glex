#pragma once
#include "Core/Container/delegate.h"
#include "Core/Container/optional.h"
#include <string.h>

namespace glex
{
	// Windows keycode.
	class KeyCode : private StaticClass
	{
	public:
		enum : uint8_t
		{
			LMB = 0x01,
			RMB,
			MMB = 0x04,
			Backspace = 0x08,
			Tab,
			Return = 0x0d,
			CapsLock = 0x14,
			Escape = 0x1b,
			Space = 0x20,
			PageUp,
			PageDown,
			End,
			Home,
			Left,
			Up,
			Right,
			Down,
			PrintScreen = 0x2c,
			Insert,
			Delete,
			Key0 = 0x30,
			Key1,
			Key2,
			Key3,
			Key4,
			Key5,
			Key6,
			Key7,
			Key8,
			Key9,
			A = 0x41,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			LeftWindows,
			RightWindows,
			Menu, // VK_APPS
			Num0 = 0x60,
			Num1,
			Num2,
			Num3,
			Num4,
			Num5,
			Num6,
			Num7,
			Num8,
			Num9,
			NumMultiply,
			NumAdd,
			NumReturn, // VK_SEPARATOR
			NumSubtract,
			NumDecimal,
			NumDivide,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			NumLock = 0x90,
			ScrollLock,
			LeftShift = 0xa0,
			RightShift,
			LeftControl,
			RightControl,
			LeftAlt,
			RightAlt,
			Semicolon = 0xba,
			Equal,
			Comma,
			Minus,
			Period,
			Slash,
			GraveAccent,
			LeftBracket = 0xdb,
			Backslash,
			RightBracket,
			Apostrophe
		};
	};

	class Input : private StaticClass
	{
	private:
		inline static bool s_state[256], s_lastFrame[256];
		inline static float s_dx, s_dy, s_x, s_y, s_scroll;
		inline static Delegate<void(uint8_t, bool)> s_keyEventDelegate; // Params: keycode, pressed.
		
#if GLEX_REPORT_MEMORY_LEAKS
		inline static Optional<Deque<uint32_t>> s_inputQueue__;
		inline static Deque<uint32_t>& s_inputQueue = *s_inputQueue__;
#else
		inline static Deque<uint32_t> s_inputQueue;
#endif

	public:
		constexpr static uint32_t INPUT_CHAR_FLAG = 0x80000000;
		constexpr static uint32_t KEY_INPUT_END = 0xffffffff;

		// GLFW keycodes.
		/* enum Key : int
		{
			LMB = 0, // Not actually key code.
			RMB = 1,
			MMB = 2,

			M0 = '0', M1, M2, M3, M4, M5, M6, M7, M8, M9,
			A = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

			GraveAccent = '`',
			Minus = '-',
			Equal = '=',
			Comma = ',',
			Period = '.',
			Slash = '/',
			Space = ' ',
			LeftBracket = '[',
			RightBracket = ']',
			Backslash = '\\',
			Semicolon = ';',
			Apostrophe = '\'',

			Escape = 256,
			Return,
			Tab,
			Backspace,
			Insert,
			Delete,
			Right,
			Down,
			Left,
			Up,
			PageUp,
			PageDown,
			Home,
			End,

			CapsLock = 280,
			ScrollLock,
			NumLock,
			PrintScreen,
			PauseBreak,

			F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

			Num0 = 320, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
			NumDot, NumDivide, NumMultiply, NumMinus, NumAdd, NumReturn,

			LeftShift = 340,
			LeftControl,
			LeftAlt,
			LeftWindows,
			RightShift,
			RightControl,
			RightAlt,
			RightWindows,
			Menu
		}; */

		static bool IsPressing(uint8_t key) { return s_state[key]; }
		static bool HasPressed(uint8_t key) { return s_state[key] && !s_lastFrame[key]; }
		static bool HasReleased(uint8_t key) { return !s_state[key] && s_lastFrame[key]; }
		static float MouseDeltaX() { return s_dx; }
		static float MouseDeltaY() { return s_dy; }
		static float MouseX() { return s_x; }
		static float MouseY() { return s_y; }
		static float MouseScroll() { return s_scroll; }
		static auto& KeyEventDelegate() { return s_keyEventDelegate; }
		static uint32_t GetInputChar();

#ifdef GLEX_INTERNAL
		static void Startup();
		static void Shutdown();
		static void Press(uint8_t key);
		static void Update();
		static void InputChar(uint32_t code);

		static void Release(uint8_t key)
		{
			s_state[key] = false;
			s_keyEventDelegate.Broadcast(key, false);
		}

		static void Move(float x, float y)
		{
			s_dx = x - s_x;
			s_dy = y - s_y;
			s_x = x;
			s_y = y;
		}

		static void Scroll(float scroll)
		{
			s_scroll = scroll;
		}
#endif
	};
}