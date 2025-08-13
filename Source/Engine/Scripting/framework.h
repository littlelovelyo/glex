#pragma once
#include "Core/Device/window.h"
#include "Core/Device/time.h"

namespace glex::py
{
	inline uint32_t Width()
	{
		return Window::Width();
	}

	inline uint32_t Height()
	{
		return Window::Height();
	}

	inline double Time()
	{
		return Time::Current();
	}

	inline float DeltaTime()
	{
		return Time::DeltaTime();
	}
}