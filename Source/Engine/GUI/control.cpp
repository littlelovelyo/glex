#include "Engine/GUI/control.h"
#include "Engine/GUI/batch.h"

using namespace glex;
using namespace glex::ui;

void Control::InvalidateMeasure()
{
	WeakPtr<Control> p = this;
	for (;;)
	{
		p->m_measureCache = glm::vec2(NAN, NAN);
		p->m_arrangeCache = glm::vec2(NAN, NAN);
		WeakPtr<Control> parent = p->m_parent;
		if (parent == nullptr)
			break;
		p = parent;
	}
}

void Control::SetHorizontalAlignment(HorizontalAlignment horizontalAlignment)
{
	m_horizontalAlignment = horizontalAlignment;
	InvalidateMeasure();
}

void Control::SetVerticalAlignment(VerticalAlignment verticalAlignment)
{
	m_verticalAlignment = verticalAlignment;
	InvalidateMeasure();
}

void Control::SetVisibility(Visibility visibility)
{
	if (m_visibility == visibility)
		return;
	Visibility oldVisibility = m_visibility;
	m_visibility = visibility;
	if (oldVisibility == Visibility::Collapsed || visibility == Visibility::Collapsed)
		InvalidateMeasure();
}

void Control::SetMargin(glm::vec4 const& margin)
{
	m_margin = margin;
	InvalidateMeasure();
}

glm::vec2 Control::Measure(glm::vec2 availableSize)
{
	if (m_visibility == Visibility::Collapsed)
		return glm::vec2(0.0f, 0.0f);

	glm::vec2 padding = glm::vec2(m_margin.x + m_margin.z, m_margin.y + m_margin.w);
	if (m_measureCache != availableSize)
	{
		m_desiredSize = OnMeasure(availableSize - padding);
		m_measureCache = availableSize;
	}
	return m_desiredSize + padding;
}

void Control::Arrange(glm::vec2 pos, glm::vec2 actualSize)
{
	if (m_visibility == Visibility::Collapsed)
		return;

	switch (m_horizontalAlignment)
	{
		case HorizontalAlignment::Left:
		{
			m_actualSize.x = m_desiredSize.x;
			m_position.x = m_margin.x;
			break;
		}
		case HorizontalAlignment::Center:
		{
			m_actualSize.x = m_desiredSize.x;
			m_position.x = (actualSize.x - m_actualSize.x - m_margin.x - m_margin.z) * 0.5f;
			break;
		}
		case HorizontalAlignment::Right:
		{
			m_actualSize.x = m_desiredSize.x;
			m_position.x = actualSize.x - m_actualSize.x - m_margin.z;
			break;
		}
		default:
		{
			m_position.x = m_margin.x;
			m_actualSize.x = glm::max(m_desiredSize.x, actualSize.x - m_margin.x - m_margin.z);
			break;
		}
	}
	switch (m_verticalAlignment)
	{
		case VerticalAlignment::Top:
		{
			m_actualSize.y = m_desiredSize.y;
			m_position.y = m_margin.y;
			break;
		}
		case VerticalAlignment::Center:
		{
			m_actualSize.y = m_desiredSize.y;
			m_position.y = (actualSize.y - m_actualSize.y - m_margin.y - m_margin.w) * 0.5f;
			break;
		}
		case VerticalAlignment::Bottom:
		{
			m_actualSize.y = m_desiredSize.y;
			m_position.y = actualSize.y - m_actualSize.y - m_margin.w;
			break;
		}
		default:
		{
			m_position.y = m_margin.y;
			m_actualSize.y = glm::max(m_desiredSize.y, actualSize.y - m_margin.y - m_margin.w);
			break;
		}
	}
	m_position += pos;

	if (actualSize != m_arrangeCache)
	{
		OnArrange();
		m_arrangeCache = actualSize;
	}
}

glm::vec2 ContentControl::OnMeasure(glm::vec2 availableSize)
{
	return m_child == nullptr ? glm::vec2(0.0f, 0.0f) : m_child->Measure(glm::vec2(0.0f, 0.0f));
}

void ContentControl::OnArrange()
{
	if (m_child != nullptr)
		m_child->Arrange(glm::vec2(0.0f, 0.0f), m_actualSize);
}

UniquePtr<Control> ContentControl::SetChild(UniquePtr<Control> newChild)
{
	UniquePtr<Control> old = std::move(m_child);
	m_child = std::move(newChild);
	if (m_child != nullptr)
		m_child->SetParent(this);
	if (old != nullptr)
		old->SetParent(nullptr);
	InvalidateMeasure();
	return old;
}