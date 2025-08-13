#include "slider.h"
#include "Renderer/renderer.h"
#include "Context/input.h"

using namespace glex;

void Slider::Draw(glm::vec2 pos, float alpha)
{
	glm::vec2 position = pos + m_finalPosition + glm::vec2(m_margin.x, m_margin.y);
	Renderer::DrawQuad(glm::vec4(position, m_finalSize), glm::vec4(m_backgroundColor, alpha));

	glm::vec2 mouse = glm::vec2(Input::MouseX(), Input::MouseY());

	if (s_mouseOwner == this)
	{
		if (Input::Released(Input::LMB))
			s_mouseOwner = nullptr;
		float thumbPosition = glm::clamp(mouse.x - position.x - m_offset, 0.0f, m_finalSize.x - k_thumbWidth);
		glm::vec4 thumbBorder = glm::vec4(position.x + thumbPosition, position.y, k_thumbWidth, m_finalSize.y);
		Renderer::DrawQuad(thumbBorder, glm::vec4(m_foregroundColor, alpha));
		float value = thumbPosition / (m_finalSize.x - k_thumbWidth) * (m_range.y - m_range.x) + m_range.x;
		if (value != m_value)
		{
			m_value = value;
			if (m_valueChanged != nullptr)
				m_valueChanged(value);
		}
	}
	else
	{
		float thumbPosition = (m_finalSize.x - k_thumbWidth) / (m_range.y - m_range.x) * (m_value - m_range.x);
		glm::vec4 thumbBorder = glm::vec4(position.x + thumbPosition, position.y, k_thumbWidth, m_finalSize.y);
		Renderer::DrawQuad(thumbBorder, glm::vec4(m_foregroundColor, alpha));
		if (mouse.x >= thumbBorder.x && mouse.y >= thumbBorder.y && mouse.x <= thumbBorder.x + k_thumbWidth && mouse.y <= thumbBorder.y + m_finalSize.y)
		{
			if (Input::Pressed(Input::LMB))
			{
				s_mouseOwner = this;
				m_offset = mouse.x - thumbBorder.x;
			}
		}
	}
}