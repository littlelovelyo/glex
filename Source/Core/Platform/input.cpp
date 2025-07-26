#include "Core/Platform/input.h"

using namespace glex;

void Input::Press(uint8_t key)
{
	s_state[key] = true;
	s_keyEventDelegate.Broadcast(key, true);
	s_inputQueue.push_back(key);
}

void Input::InputChar(uint32_t code)
{
	s_inputQueue.push_back(code | INPUT_CHAR_FLAG);
}

uint32_t Input::GetInputChar()
{
	if (s_inputQueue.empty())
		return KEY_INPUT_END;
	uint32_t ret = s_inputQueue.front();
	s_inputQueue.pop_front();
	return ret;
}

void Input::Update()
{
	memcpy(s_lastFrame, s_state, 256);
	s_dx = 0.0f;
	s_dy = 0.0f;
	s_scroll = 0.0f;
	s_inputQueue.clear();
}

void Input::Startup()
{
#if GLEX_REPORT_MEMORY_LEAKS
	s_inputQueue__.Emplace();
#endif
}

void Input::Shutdown()
{
#if GLEX_REPORT_MEMORY_LEAKS
	s_keyEventDelegate.Clear();
	s_inputQueue__.Destroy();
#endif
}