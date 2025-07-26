#pragma once
#include <utility>
#include "Engine/Scripting/module.h"

namespace glex
{
	class Scene;

	class Script
	{
	private:
		PyModule m_module;
		PyFunction<void()> m_start, m_update;

	public:
		Script(PyModule module);
		void SetScript(PyModule module);

	};
}