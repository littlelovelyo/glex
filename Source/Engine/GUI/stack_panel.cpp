#include "Engine/GUI/stack_panel.h"
#include "Engine/GUI/batch.h"

using namespace glex;
using namespace glex::ui;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Vertical Box
————————————————————————————————————————————————————————————————————————————————————————————————————*/
glm::vec2 VerticalBox::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 size = glm::vec2(0.0f, 0.0f);
	for (UniquePtr<Control> const& child : m_children)
	{
		glm::vec2 childSize = child->Measure(glm::vec2(0.0f, 0.0f));
		size.x = glm::max(size.x, childSize.x);
		size.y += childSize.y;
	}
	return size;
}

void VerticalBox::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	for (UniquePtr<Control> const& child : m_children)
	{
		glm::vec2 const& rect = child->GetDesiredSize();
		glm::vec4 const& margin = child->GetMargin();
		float height = rect.y + margin.y + margin.w;
		child->Arrange(pos, glm::vec2(m_actualSize.x, height));
		pos.y += height;
	}
}

void VerticalBox::PushChild(UniquePtr<Control> control)
{
	control->SetParent(this);
	m_children.emplace_back(std::move(control));
	InvalidateMeasure();
}

UniquePtr<Control> VerticalBox::PopChild()
{
	UniquePtr<Control> child = std::move(m_children.back());
	m_children.pop_back();
	InvalidateMeasure();
	return child;
}

void VerticalBox::PopChild(uint32_t count)
{
	m_children.resize(m_children.size() - count);
	InvalidateMeasure();
}

UniquePtr<Control> VerticalBox::RemoveChild(uint32_t index)
{
	UniquePtr<Control> child = std::move(m_children[index]);
	m_children.erase(m_children.begin() + index);
	InvalidateMeasure();
	return child;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Horizontal Box
————————————————————————————————————————————————————————————————————————————————————————————————————*/
glm::vec2 HorizontalBox::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 size = glm::vec2(0.0f, 0.0f);
	for (UniquePtr<Control> const& child : m_children)
	{
		glm::vec2 childSize = child->Measure(glm::vec2(0.0f, 0.0f));
		size.y = glm::max(size.y, childSize.y);
		size.x += childSize.x;
	}
	return size;
}

void HorizontalBox::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	for (UniquePtr<Control> const& child : m_children)
	{
		glm::vec2 const& rect = child->GetDesiredSize();
		glm::vec4 const& margin = child->GetMargin();
		float width = rect.x + margin.x + margin.z;
		child->Arrange(pos, glm::vec2(width, m_actualSize.y));
		pos.x += width;
	}
}

void HorizontalBox::PushChild(UniquePtr<Control> control)
{
	control->SetParent(this);
	m_children.emplace_back(std::move(control));
	InvalidateMeasure();
}

UniquePtr<Control> HorizontalBox::PopChild()
{
	UniquePtr<Control> child = std::move(m_children.back());
	m_children.pop_back();
	InvalidateMeasure();
	return child;
}

void HorizontalBox::PopChild(uint32_t count)
{
	m_children.resize(m_children.size() - count);
	InvalidateMeasure();
}

UniquePtr<Control> HorizontalBox::RemoveChild(uint32_t index)
{
	UniquePtr<Control> child = std::move(m_children[index]);
	m_children.erase(m_children.begin() + index);
	InvalidateMeasure();
	return child;
}