#include "Engine/GUI/border.h"
#include "Engine/GUI/batch.h"

using namespace glex;
using namespace glex::ui;

void Border::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(pos, size), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), m_color);
}