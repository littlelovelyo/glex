#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class ColorBlock : public Control
	{
	private:
		glm::vec2 m_size;
		glm::vec4 m_color;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		void SetSize(glm::vec2 size) { m_size = size; InvalidateMeasure(); }
		void SetColor(glm::vec4 const& color) { m_color = color; }
	};
}