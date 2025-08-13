#include "Engine/GUI/button.h"
#include "Engine/GUI/batch.h"

using namespace glex::ui;

Button::Button()
{
	m_acceptMouseEvent = true;
	UniquePtr<TextBlock> textBlock = MakeUnique<TextBlock>();
	SetChild(std::move(textBlock));
}

void Button::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	constexpr glm::vec4 HOVER_COLOR = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	constexpr glm::vec4 NORMAL_COLOR = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	BatchRenderer::DrawQuad(glm::vec4(pos, size), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), m_hovered ? HOVER_COLOR : NORMAL_COLOR);
}

void Button::OnMouseEvent(MouseEvent event, glm::vec2 pos)
{
	switch (event)
	{
		case MouseEvent::Enter: m_hovered = true; break;
		case MouseEvent::Leave: m_hovered = false; break;
		case MouseEvent::Release: if (m_click != nullptr) m_click(pos); break;
	}
}