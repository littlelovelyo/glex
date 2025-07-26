#include "Engine/GUI/scrollbox.h"
#include "Core/Platform/input.h"

using namespace glex;
using namespace glex::ui;

ScrollBox::ScrollBox()
{
	m_acceptMouseEvent = true;
}

glm::vec2 ScrollBox::OnMeasure(glm::vec2 availableSize)
{
	if (m_child == nullptr)
		return glm::vec2(0.0f, 0.0f);
	glm::vec2 childSize = m_child->Measure(glm::vec2(availableSize.x, 0.0f));
	return childSize;
}

void ScrollBox::OnArrange()
{
	if (m_child != nullptr)
		m_child->Arrange(glm::vec2(0.0f, -m_offset), m_actualSize);
}

void ScrollBox::OnMouseEvent(MouseEvent event, glm::vec2 mousePos)
{
	if (event == MouseEvent::Scroll)
	{
		if (m_child != nullptr)
		{
			m_offset -= Input::MouseScroll();
			glm::vec4 const& rect = m_child->GetRect();
			glm::vec4 const& padding = m_child->GetMargin();
			m_offset = glm::clamp(m_offset, 0.0f, glm::max(0.0f, rect.w - padding.y - padding.w));
			m_child->Arrange(glm::vec2(0.0f, -m_offset), m_actualSize);
		}
	}
}