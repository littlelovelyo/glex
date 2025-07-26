#include "Core/Platform/time.h"
#include <GLFW/glfw3.h>
using namespace glex;

void Time::Startup()
{
	s_deltaTime = 0.0f;
	s_time = glfwGetTime() * 1000.0;
}

void Time::Update()
{
	double time = glfwGetTime() * 1000.0;
	s_deltaTime = time - s_time;
	s_time = time;
}