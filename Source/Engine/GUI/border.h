#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class Border : public ContentControl
	{
	private:
		glm::vec4 m_color;

	public:
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size);
		void SetColor(glm::vec4 const& color) { m_color = color; }
	};
}