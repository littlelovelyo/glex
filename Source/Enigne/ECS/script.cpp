#include "Engine/ECS/script.h"
#include "config.h"

using namespace glex;
using namespace glex::py;

Script::Script(PyModule module) : m_module(module)
{
	m_start = module.GetFunction<void()>("start");
	m_update = module.GetFunction<void()>("update");
}