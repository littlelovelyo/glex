#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class SizeBox : public ContentControl
	{
	private:
		glm::vec2 m_size;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		void SetSize(glm::vec2 size) { m_size = size; InvalidateMeasure(); }
	};
}