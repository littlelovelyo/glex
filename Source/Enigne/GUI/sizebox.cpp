#include "Engine/GUI/sizebox.h"

using namespace glex::ui;

glm::vec2 SizeBox::OnMeasure(glm::vec2 availableSize)
{
	m_child->Measure(m_size);
	return m_size;
}