#include "Engine/GUI/listbox.h"
#include "Engine/GUI/batch.h"
#include "Core/Platform/input.h"
#include "Core/Platform/time.h"

using namespace glex;
using namespace glex::ui;

ListBox::ListBox(SharedPtr<MaterialInstance> const& material, SharedPtr<BitmapFont> const& font)
{
	m_acceptMouseEvent = true;
	SetMaterial(material);
	m_font = font;
}

glm::vec2 ListBox::OnMeasure(glm::vec2 availableSize)
{
	return glm::vec2(0.0f, 0.0f);
}

void ListBox::OnArrange()
{
	CalculateStuff();
}

void ListBox::CalculateStuff()
{
	float fullHeight = m_textSize * m_font->LineHeight() * m_items.size();
	float maxOffset = glm::max(0.0f, fullHeight - m_actualSize.y);
	m_offset = glm::clamp(m_offset, 0.0f, maxOffset);
}

void ListBox::AddItem(String text, SharedPtr<MaterialInstance> const& textMaterial)
{
	UniquePtr<TextBlock> textBlock = MakeUnique<TextBlock>();
	textBlock->SetMaterial(textMaterial);
	textBlock->SetFont(m_font);
	textBlock->SetTextSize(m_textSize);
	textBlock->SetText(std::move(text));
	m_items.push_back(std::move(textBlock));
	CalculateStuff();
}

void ListBox::RemoveItem(uint32_t index)
{
	m_items.erase(m_items.begin() + index);
	CalculateStuff();
	m_hoverIndex = -1;
	m_selectIndex = -1;
}

String const& ListBox::GetItemText(uint32_t index)
{
	return m_items[index]->GetText();
}

void ListBox::Clear()
{
	m_items.clear();
	CalculateStuff();
	m_hoverIndex = -1;
	m_selectIndex = -1;
}

uint32_t ListBox::NumChildren() const
{
	float itemHeight = m_font->LineHeight() * m_textSize;
	uint32_t startIndex = m_offset / itemHeight;
	uint32_t endIndex = glm::ceil(m_offset + m_actualSize.y / itemHeight);
	endIndex = glm::min(endIndex, m_items.size());
	return endIndex - startIndex;
}

WeakPtr<Control> ListBox::GetChild(uint32_t index)
{
	float itemHeight = m_font->LineHeight() * m_textSize;
	index += m_offset / itemHeight;
	WeakPtr<Control> item = m_items[index];
	item->Arrange(glm::vec2(0.0f, index * itemHeight - m_offset), glm::vec2(m_actualSize.x, itemHeight));
	return item;
}

void ListBox::SetSelection(uint32_t index)
{
	if (index != -1 && index >= m_items.size())
		m_selectIndex = -1;
	else
		m_selectIndex = index;
}

void ListBox::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	if (m_selectIndex != -1)
	{
		float itemHeight = m_font->LineHeight() * m_textSize;
		float y = m_selectIndex * itemHeight - m_offset;
		if (y > -itemHeight && y < size.y)
		{
			y += pos.y;
			BatchRenderer::DrawQuad(glm::vec4(pos.x, y, size.x, itemHeight), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.4f, 0.4f, 0.6f, 1.0f));
		}
	}
	if (m_hoverIndex != -1 && m_hoverIndex != m_selectIndex)
	{
		float itemHeight = m_font->LineHeight() * m_textSize;
		float y = m_hoverIndex * itemHeight - m_offset + pos.y;
		BatchRenderer::DrawQuad(glm::vec4(pos.x, y, size.x, itemHeight), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
	}
}

void ListBox::OnMouseEvent(MouseEvent event, glm::vec2 pos)
{
	switch (event)
	{
		case MouseEvent::Enter:
		case MouseEvent::Move:
		{
			float itemHeight = m_font->LineHeight() * m_textSize;
			m_hoverIndex = (m_offset + pos.y) / itemHeight;
			if (m_hoverIndex >= m_items.size())
				m_hoverIndex = -1;
			break;
		}
		case MouseEvent::Leave:
		{
			m_hoverIndex = -1;
			break;
		}
		case MouseEvent::Press:
		{
			m_selectIndex = m_hoverIndex;
			break;
		}
		case MouseEvent::DoubleClick:
		{
			if (m_onDoubleClick != nullptr)
				m_onDoubleClick(m_selectIndex);
			break;
		}
		case MouseEvent::Scroll:
		{
			m_offset -= Input::MouseScroll() * m_textSize * m_font->LineHeight();
			CalculateStuff();
			m_hoverIndex = -1;
			break;
		}
	}
}