#include "Engine/GUI/two_slice.h"

using namespace glex;
using namespace glex::ui;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Vertical Slice
————————————————————————————————————————————————————————————————————————————————————————————————————*/
VerticalTwoSlices::VerticalTwoSlices()
{
	m_horizontalAlignment = HorizontalAlignment::Stretch;
	m_verticalAlignment = VerticalAlignment::Stretch;
}

UniquePtr<Control> VerticalTwoSlices::SetUpper(UniquePtr<Control> control)
{
	UniquePtr<Control> result = std::move(m_upper);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_upper = std::move(control);
	if (m_upper != nullptr)
		m_upper->SetParent(this);
	InvalidateMeasure();
	return result;
}

UniquePtr<Control> VerticalTwoSlices::SetLower(UniquePtr<Control> control)
{
	UniquePtr<Control> result = std::move(m_lower);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_lower = std::move(control);
	if (m_lower != nullptr)
		m_lower->SetParent(this);
	InvalidateMeasure();
	return result;
}

glm::vec2 VerticalTwoSlices::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 upperSize = m_upper != nullptr ? m_upper->Measure(glm::vec2(availableSize.x, 0.0f)) : glm::vec2(0.0f, 0.0f);
	glm::vec2 lowerSize = m_lower != nullptr ? m_lower->Measure(glm::vec2(availableSize.x, availableSize.y - upperSize.y)) : glm::vec2(0.0f, 0.0f);
	return glm::vec2(Max(availableSize.x, upperSize.x, lowerSize.x), glm::max(availableSize.y, upperSize.y + lowerSize.y));
}

void VerticalTwoSlices::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	if (m_upper != nullptr)
	{
		glm::vec2 const& upperRect = m_upper->GetDesiredSize();
		glm::vec4 const& margin = m_upper->GetMargin();
		float height = upperRect.y + margin.y + margin.w;
		m_upper->Arrange(pos, glm::vec2(m_actualSize.x, height));
		pos.y += height;
	}
	if (m_lower != nullptr)
		m_lower->Arrange(pos, glm::vec2(m_actualSize.x, m_actualSize.y - pos.y));
}

uint32_t VerticalTwoSlices::NumChildren() const
{
	uint32_t result = 0;
	if (m_upper != nullptr)
		result++;
	if (m_lower != nullptr)
		result++;
	return result;
}

WeakPtr<Control> VerticalTwoSlices::GetChild(uint32_t index)
{
	return index == 1 || m_upper == nullptr ? m_lower : m_upper;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Horizontal Slice
————————————————————————————————————————————————————————————————————————————————————————————————————*/
HorizontalTwoSlices::HorizontalTwoSlices()
{
	m_horizontalAlignment = HorizontalAlignment::Stretch;
	m_verticalAlignment = VerticalAlignment::Stretch;
}

UniquePtr<Control> HorizontalTwoSlices::SetLeft(UniquePtr<Control> control)
{
	UniquePtr<Control> result = std::move(m_left);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_left = std::move(control);
	if (m_left != nullptr)
		m_left->SetParent(this);
	InvalidateMeasure();
	return result;
}

UniquePtr<Control> HorizontalTwoSlices::SetRight(UniquePtr<Control> control)
{
	UniquePtr<Control> result = std::move(m_right);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_right = std::move(control);
	if (m_right != nullptr)
		m_right->SetParent(this);
	InvalidateMeasure();
	return result;
}

glm::vec2 HorizontalTwoSlices::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 leftSize = m_left != nullptr ? m_left->Measure(glm::vec2(0.0f, availableSize.y)) : glm::vec2(0.0f, 0.0f);
	glm::vec2 rightSize = m_right != nullptr ? m_right->Measure(glm::vec2(availableSize.x - leftSize.x, availableSize.y)) : glm::vec2(0.0f, 0.0f);
	return glm::vec2(glm::max(availableSize.x, leftSize.x + rightSize.x), Max(availableSize.y, leftSize.y, rightSize.y));
}

void HorizontalTwoSlices::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	if (m_left != nullptr)
	{
		glm::vec2 const& upperRect = m_left->GetDesiredSize();
		glm::vec4 const& margin = m_left->GetMargin();
		float width = upperRect.x + margin.x + margin.z;
		m_left->Arrange(pos, glm::vec2(width, m_actualSize.y));
		pos.x += width;
	}
	if (m_right != nullptr)
		m_right->Arrange(pos, glm::vec2(m_actualSize.x - pos.x, m_actualSize.y));
}

uint32_t HorizontalTwoSlices::NumChildren() const
{
	uint32_t result = 0;
	if (m_left != nullptr)
		result++;
	if (m_right != nullptr)
		result++;
	return result;
}

WeakPtr<Control> HorizontalTwoSlices::GetChild(uint32_t index)
{
	return index == 1 || m_left == nullptr ? m_right : m_left;
}