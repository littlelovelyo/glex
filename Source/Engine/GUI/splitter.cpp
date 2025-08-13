#include "Engine/GUI/splitter.h"
#include "Engine/GUI/batch.h"
#include "Core/Platform/window.h"
#include "Core/Platform/input.h"

using namespace glex;
using namespace glex::ui;

constexpr float SPLITTER_WIDTH = 4.0f;
constexpr float SPLITTER_LINE_OFFSET = 1.0f;
constexpr float SPLITTER_LINE_WIDTH = 2.0f;
static float s_initialMousePosition = 0.0f;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Horizontal Splitter
————————————————————————————————————————————————————————————————————————————————————————————————————*/
HorizontalSplitter::HorizontalSplitter()
{
	m_acceptMouseEvent = true;
}

UniquePtr<Control> HorizontalSplitter::SetLeft(UniquePtr<Control>&& left)
{
	UniquePtr<Control> result = std::move(m_left);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_left = std::move(left);
	if (m_left != nullptr)
		m_left->SetParent(this);
	InvalidateMeasure();
	return result;
}

UniquePtr<Control> HorizontalSplitter::SetRight(UniquePtr<Control>&& right)
{
	UniquePtr<Control> result = std::move(m_right);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_right = std::move(right);
	if (m_right != nullptr)
		m_right->SetParent(this);
	InvalidateMeasure();
	return result;
}

glm::vec2 HorizontalSplitter::OnMeasure(glm::vec2 availableSize)
{
	float originalWidth = m_actualSize.x - SPLITTER_WIDTH;
	if (originalWidth > 0.0f)
		m_splitterPosition = m_splitterPosition / originalWidth * (availableSize.x - SPLITTER_WIDTH);
	m_splitterPosition = glm::clamp(m_splitterPosition, 0.0f, availableSize.x - SPLITTER_WIDTH);
	if (m_left != nullptr)
		m_left->Measure(glm::vec2(m_splitterPosition, availableSize.y));
	if (m_right != nullptr)
		m_right->Measure(glm::vec2(availableSize.x - m_splitterPosition - SPLITTER_WIDTH, availableSize.y));
	return availableSize;
}

void HorizontalSplitter::OnArrange()
{
	if (m_left != nullptr)
		m_left->Arrange(glm::vec2(0.0f, 0.0f), glm::vec2(m_splitterPosition, m_actualSize.y));
	if (m_right != nullptr)
		m_right->Arrange(glm::vec2(m_splitterPosition + SPLITTER_WIDTH, 0.0f), glm::vec2(m_actualSize.x - m_splitterPosition - SPLITTER_WIDTH, m_actualSize.y));
}

uint32_t HorizontalSplitter::NumChildren() const
{
	uint32_t result = 0;
	if (m_left != nullptr)
		result++;
	if (m_right != nullptr)
		result++;
	return result;
}

WeakPtr<Control> HorizontalSplitter::GetChild(uint32_t index)
{
	return index == 1 || m_left == nullptr ? m_right : m_left;
}

void HorizontalSplitter::OnMouseEvent(MouseEvent event, glm::vec2 mousePos)
{
	switch (event)
	{
		case MouseEvent::Enter:
		{
			if (mousePos.x >= m_splitterPosition && mousePos.x <= m_splitterPosition + SPLITTER_WIDTH)
				Window::SetCursor(Cursor::HorizontalResize);
			break;
		}
		case MouseEvent::Move:
		{
			Window::SetCursor(mousePos.x >= m_splitterPosition && mousePos.x <= m_splitterPosition + SPLITTER_WIDTH ? Cursor::HorizontalResize : Cursor::Arrow);
			break;
		}
		case MouseEvent::Leave:
		{
			Window::SetCursor(Cursor::Arrow);
			break;
		}
		case MouseEvent::Press:
		{
			if (mousePos.x >= m_splitterPosition && mousePos.x <= m_splitterPosition + SPLITTER_WIDTH)
				s_initialMousePosition = Input::MouseX();
			else
				s_initialMousePosition = NAN;
			break;
		}
		case MouseEvent::Release:
		case MouseEvent::Abort:
		{
			if (!isnan(s_initialMousePosition))
			{
				m_splitterPosition += Input::MouseX() - s_initialMousePosition;
				m_splitterPosition = glm::clamp(m_splitterPosition, 0.0f, m_actualSize.x - SPLITTER_WIDTH);
				if (m_left != nullptr)
				{
					m_left->Measure(glm::vec2(m_splitterPosition, m_actualSize.y));
					m_left->Arrange(glm::vec2(0.0f, 0.0f), glm::vec2(m_splitterPosition, m_actualSize.y));
				}
				if (m_right != nullptr)
				{
					m_right->Measure(glm::vec2(m_actualSize.x - m_splitterPosition - SPLITTER_WIDTH, m_actualSize.y));
					m_right->Arrange(glm::vec2(m_splitterPosition + SPLITTER_WIDTH, 0.0f), glm::vec2(m_actualSize.x - m_splitterPosition - SPLITTER_WIDTH, m_actualSize.y));;
				}
			}
			break;
		}
	}
}

void HorizontalSplitter::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(pos.x + m_splitterPosition + SPLITTER_LINE_OFFSET, pos.y, SPLITTER_LINE_WIDTH, size.y), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Vertical Splitter
————————————————————————————————————————————————————————————————————————————————————————————————————*/
VerticalSplitter::VerticalSplitter()
{
	m_acceptMouseEvent = true;
}

UniquePtr<Control> VerticalSplitter::SetUpper(UniquePtr<Control>&& upper)
{
	UniquePtr<Control> result = std::move(m_upper);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_upper = std::move(upper);
	if (m_upper != nullptr)
		m_upper->SetParent(this);
	InvalidateMeasure();
	return result;
}

UniquePtr<Control> VerticalSplitter::SetLower(UniquePtr<Control>&& lower)
{
	UniquePtr<Control> result = std::move(m_lower);
	if (result != nullptr)
		result->SetParent(nullptr);
	m_lower = std::move(lower);
	if (m_lower != nullptr)
		m_lower->SetParent(this);
	InvalidateMeasure();
	return result;
}

glm::vec2 VerticalSplitter::OnMeasure(glm::vec2 availableSize)
{
	float originalHeight = m_actualSize.y - SPLITTER_WIDTH;
	if (originalHeight > 0.0f)
		m_splitterPosition = m_splitterPosition / originalHeight * (availableSize.y - SPLITTER_WIDTH);
	m_splitterPosition = glm::clamp(m_splitterPosition, 0.0f, availableSize.y - SPLITTER_WIDTH);
	if (m_upper != nullptr)
		m_upper->Measure(glm::vec2(availableSize.x, m_splitterPosition));
	if (m_lower != nullptr)
		m_lower->Measure(glm::vec2(availableSize.x, availableSize.y - m_splitterPosition - SPLITTER_WIDTH));
	return availableSize;
}

void VerticalSplitter::OnArrange()
{
	if (m_upper != nullptr)
		m_upper->Arrange(glm::vec2(0.0f, 0.0f), glm::vec2(m_actualSize.x, m_splitterPosition));
	if (m_lower != nullptr)
		m_lower->Arrange(glm::vec2(0.0f, m_splitterPosition + SPLITTER_WIDTH), glm::vec2(m_actualSize.x, m_actualSize.y - m_splitterPosition - SPLITTER_WIDTH));
}

uint32_t VerticalSplitter::NumChildren() const
{
	uint32_t result = 0;
	if (m_upper != nullptr)
		result++;
	if (m_lower != nullptr)
		result++;
	return result;
}

WeakPtr<Control> VerticalSplitter::GetChild(uint32_t index)
{
	return index == 1 || m_upper == nullptr ? m_lower : m_upper;
}

void VerticalSplitter::OnMouseEvent(MouseEvent event, glm::vec2 mousePos)
{
	switch (event)
	{
		case MouseEvent::Enter:
		{
			if (mousePos.y >= m_splitterPosition && mousePos.y <= m_splitterPosition + SPLITTER_WIDTH)
				Window::SetCursor(Cursor::VerticalResize);
			break;
		}
		case MouseEvent::Move:
		{
			Window::SetCursor(mousePos.y >= m_splitterPosition && mousePos.y <= m_splitterPosition + SPLITTER_WIDTH ? Cursor::VerticalResize : Cursor::Arrow);
			break;
		}
		case MouseEvent::Leave:
		{
			Window::SetCursor(Cursor::Arrow);
			break;
		}
		case MouseEvent::Press:
		{
			if (mousePos.y >= m_splitterPosition && mousePos.y <= m_splitterPosition + SPLITTER_WIDTH)
				s_initialMousePosition = Input::MouseY();
			else
				s_initialMousePosition = NAN;
			break;
		}
		case MouseEvent::Release:
		case MouseEvent::Abort:
		{
			if (!isnan(s_initialMousePosition))
			{
				m_splitterPosition += Input::MouseY() - s_initialMousePosition;
				m_splitterPosition = glm::clamp(m_splitterPosition, 0.0f, m_actualSize.y - SPLITTER_WIDTH);
				if (m_upper != nullptr)
				{
					m_upper->Measure(glm::vec2(m_actualSize.x, m_splitterPosition));
					m_upper->Arrange(glm::vec2(0.0f, 0.0f), glm::vec2(m_actualSize.x, m_splitterPosition));
				}
				if (m_lower != nullptr)
				{
					m_lower->Measure(glm::vec2(m_actualSize.x, m_actualSize.y - m_splitterPosition - SPLITTER_WIDTH));
					m_lower->Arrange(glm::vec2(0.0f, m_splitterPosition + SPLITTER_WIDTH), glm::vec2(m_actualSize.x, m_actualSize.y - m_splitterPosition - SPLITTER_WIDTH));
				}
			}
			break;
		}
	}
}

void VerticalSplitter::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y + m_splitterPosition + SPLITTER_LINE_OFFSET, size.x, SPLITTER_LINE_WIDTH), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}