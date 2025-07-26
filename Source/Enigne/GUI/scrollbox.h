#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class ScrollBox : public ui::ContentControl
	{
	private:
		float m_offset = 0.0f;

	public:
		ScrollBox();
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 mousePos) override;
	};
}