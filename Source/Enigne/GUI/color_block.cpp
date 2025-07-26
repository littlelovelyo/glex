#include "Engine/GUI/color_block.h"
#include "Engine/GUI/batch.h"

using namespace glex;
using namespace glex::ui;

glm::vec2 ColorBlock::OnMeasure(glm::vec2 availableSize)
{
	return m_size;
}

void ColorBlock::OnPaint(glm::vec2 position, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(position, size), nullptr, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_color);
}