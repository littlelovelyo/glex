#include "Engine/GUI/treeview.h"
#include "Engine/GUI/batch.h"

using namespace glex;
using namespace glex::ui;

constexpr float INDENT = 12.0f;

TreeViewItem::TreeViewItem(WeakPtr<TreeView> owner, SharedPtr<MaterialInstance> const& material, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font)
	: m_owner(owner), m_userData(nullptr), m_expanded(false), m_hovered(false)
{
	m_acceptMouseEvent = true;
	SetMaterial(material);
	m_header.SetMaterial(textMaterial);
	m_header.SetFont(font);
}

glm::vec2 TreeViewItem::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 actualSize(availableSize.x, 0.0f);
	actualSize.y += m_header.Measure(glm::vec2(availableSize.x, 0.0f)).y;
	if (m_expanded)
	{
		for (UniquePtr<TreeViewItem>& item : m_children)
			actualSize.y += item->Measure(glm::vec2(availableSize.x - INDENT, 0.0f)).y;
	}
	return actualSize;
}

void TreeViewItem::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	glm::vec4 const& rect = m_header.GetRect();
	m_header.Arrange(pos, glm::vec2(m_actualSize.x, rect.w)); // NO MARGIN HERE.
	if (m_expanded)
	{
		pos.x += INDENT;
		pos.y += rect.w;
		for (UniquePtr<TreeViewItem>& item : m_children)
		{
			glm::vec4 const& rect = item->GetRect();
			item->Arrange(pos, glm::vec2(m_actualSize.x - INDENT, rect.w));
			pos.y += rect.w;
		}
	}
}

void TreeViewItem::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	if (m_owner->GetActiveItem() == this)
	{
		glm::vec4 const& rect = m_header.GetRect();
		BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y, size.x, rect.w), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.4f, 0.4f, 0.6f, 1.0f));
	}
	else if (m_hovered)
	{
		glm::vec4 const& rect = m_header.GetRect();
		BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y, size.x, rect.w), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
	}
}

void TreeViewItem::OnMouseEvent(MouseEvent event, glm::vec2 mousePos)
{
	switch (event)
	{
		case MouseEvent::Enter: m_hovered = true; m_owner->SetHoverItem(this); break;
		case MouseEvent::Leave: m_hovered = false; m_owner->SetHoverItem(nullptr); break;
		case MouseEvent::Press:	m_owner->SetActiveItem(this); break;
		case MouseEvent::DoubleClick: m_expanded = !m_expanded; InvalidateMeasure(); break;
		case MouseEvent::Abort: m_owner->DispatchDragEvent(this); break;
		default: break;
	}
}

void TreeViewItem::PushChild(UniquePtr<TreeViewItem> child)
{
	child->SetParent(this);
	m_children.push_back(std::move(child));
	InvalidateMeasure();
}

void TreeViewItem::RemoveItem(WeakPtr<TreeViewItem> item)
{
	auto iter = eastl::find(m_children.begin(), m_children.end(), item);
	if (iter != m_children.end())
	{
		m_children.erase(iter);
		m_owner->SetHoverItem(nullptr);
		m_owner->SetActiveItem(nullptr);
		if (m_expanded)
			InvalidateMeasure();
	}
}

glm::vec2 TreeView::OnMeasure(glm::vec2 availableSize)
{
	for (UniquePtr<TreeViewItem>& item : m_children)
		item->Measure(glm::vec2(availableSize.x, 0.0f));
	return glm::vec2(0.0f, 0.0f);
}

void TreeView::OnArrange()
{
	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	for (UniquePtr<TreeViewItem>& item : m_children)
	{
		glm::vec4 const& rect = item->GetRect();
		item->Arrange(pos, glm::vec2(m_actualSize.x, rect.w));
		pos.y += rect.w;
	}
}

void TreeView::Clear()
{
	m_children.clear();
	m_hoverItem = nullptr;
	m_activeItem = nullptr;
}

void TreeView::PushChild(UniquePtr<TreeViewItem> child)
{
	child->SetParent(this);
	m_children.push_back(std::move(child));
	InvalidateMeasure();
}

void TreeView::RemoveItem(WeakPtr<TreeViewItem> item)
{
	auto iter = eastl::find(m_children.begin(), m_children.end(), item);
	if (iter != m_children.end())
	{
		m_children.erase(iter);
		m_hoverItem = nullptr;
		m_activeItem = nullptr;
	}
	InvalidateMeasure();
}

void TreeView::DispatchDragEvent(WeakPtr<TreeViewItem> from)
{
	if (m_onDrag != nullptr && m_hoverItem != nullptr)
		m_onDrag(from, m_hoverItem);
}

void TreeView::SetActiveItem(WeakPtr<TreeViewItem> item)
{
	if (m_onSelectionChanged != nullptr)
	{
		if (!m_onSelectionChanged(m_activeItem, item))
			return;
	}
	m_activeItem = item;
}