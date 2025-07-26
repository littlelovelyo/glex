#pragma once
#include "Core/commdefs.h"

namespace glex
{
	class Time : private StaticClass
	{
	private:
		inline static double s_time;
		inline static float s_deltaTime;

	public:
		static double Current()
		{
			return s_time;
		}

		static float DeltaTime()
		{
			return s_deltaTime;
		}

#ifdef GLEX_INTERNAL
		static void Startup();
		static void Update();
#endif
	};
}