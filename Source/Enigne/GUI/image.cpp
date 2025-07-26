#include "Engine/GUI/image.h"
#include "Engine/GUI/batch.h"

using namespace glex;

glm::vec2 ui::Image::OnMeasure(glm::vec2 availableSize)
{
	if (m_image == nullptr)
		return glm::vec2(0.0f, 0.0f);
	glm::vec2 size = m_image->Size();
	float aspectRatio = availableSize.x / availableSize.y;
	float desiredAspectRatio = size.x / size.y;
	if (desiredAspectRatio <= aspectRatio)
		return glm::vec2(availableSize.y * desiredAspectRatio, availableSize.y);
	else
		return glm::vec2(availableSize.x, availableSize.x / desiredAspectRatio);
}

void ui::Image::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(pos, size), m_image, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}