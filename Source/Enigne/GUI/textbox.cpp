#include "Engine/GUI/textbox.h"
#include "Engine/GUI/batch.h"
#include "Core/Utils/raii.h"
#include "Core/Utils/string.h"
#include "Core/Platform/input.h"
#include "Core/Platform/time.h"
#include "Core/log.h"

using namespace glex::ui;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Render proxy
————————————————————————————————————————————————————————————————————————————————————————————————————*/
glm::vec2 TextBoxRenderProxy::OnMeasure(glm::vec2 availableSize)
{
	return availableSize;
}

void TextBoxRenderProxy::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	if (m_font == nullptr)
		return;
	glm::vec2 relativePos = glm::vec2(-m_offset, 0.0f);
	char const* p = m_text.c_str();
	uint32_t charCount = 0;
	while (*p != 0)
	{
		Utf8Code code = StringUtils::CodeOfUtf8(p);
		if (code.length == 0)
			break;
		BitmapGlyph const* glyph = m_font->GetGlyph(code.code);
		if (glyph != nullptr)
		{
			glm::vec4 border = glm::vec4(relativePos + glyph->offset * m_textSize, glyph->size * m_textSize);
			if (border.x >= size.x)
				break;
			if (border.x + border.z > 0.0f)
			{
				border.x += pos.x;
				border.y += pos.y;
				BatchRenderer::DrawQuad(border, m_font->GetTexture(glyph->page), glyph->uv, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}
			relativePos.x += glyph->xadvance * m_textSize;
		}
		charCount++;
		p += code.length;
	}
}


/*————————————————————————————————————————————————————————————————————————————————————————————————————
		TextBox
————————————————————————————————————————————————————————————————————————————————————————————————————*/
TextBox::TextBox(SharedPtr<BitmapFont> const& font, float size)
{
	m_proxy.m_font = font;
	m_proxy.m_textSize = size;
	m_acceptMouseEvent = true;
	m_acceptKeyboardEvent = true;
	m_proxy.SetHorizontalAlignment(HorizontalAlignment::Stretch);
}

glm::vec2 TextBox::OnMeasure(glm::vec2 availableSize)
{
	glm::vec2 selfSize = glm::vec2(2.0f, m_proxy.m_font == nullptr ? 2.0f : m_proxy.m_font->LineHeight() * m_proxy.m_textSize + 2.0f); // 1 px border.
	m_proxy.Measure(selfSize - glm::vec2(2.0f, 2.0f));
	return selfSize;
}

void TextBox::OnArrange()
{
	m_proxy.Arrange(glm::vec2(1.0f, 1.0f), m_actualSize - glm::vec2(2.0f, 2.0f));
}

void TextBox::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y, size.x, 1.0f), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y, 1.0f, size.y), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	BatchRenderer::DrawQuad(glm::vec4(pos.x + size.x - 1.0f, pos.y, 1.0f, size.y), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	BatchRenderer::DrawQuad(glm::vec4(pos.x, pos.y + size.y - 1.0f, size.x, 1.0f), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (BatchRenderer::GetFocus() == this)
	{
		if (glm::mod(Time::Current(), 1000.0) < 500.0)
			BatchRenderer::DrawQuad(glm::vec4(pos.x + m_cursorPosCache + 1.0f, pos.y + 1.0f, 1.0f, size.y - 2.0f), nullptr, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.8f, 1.0f));
	}
}

void TextBox::OnMouseEvent(MouseEvent event, glm::vec2 pos)
{
	if (event == MouseEvent::Press)
		BatchRenderer::SetFocus(this);
}

void TextBox::OnInput(KeyboardEvent event, uint32_t code)
{
	switch (event)
	{
		case KeyboardEvent::CharInput:
		{
			Utf8Char chr = StringUtils::Utf8OfCode(code);
			if (chr.length == 0)
				break;
			m_proxy.m_text.insert(m_proxy.m_text.begin() + m_cursorIndex, chr.str, chr.str + chr.length);
			m_cursorIndex += chr.length;
			BitmapGlyph const* glyph = m_proxy.m_font->GetGlyph(code);
			if (glyph != nullptr)
			{
				m_cursorPosCache += glyph->xadvance * m_proxy.m_textSize;
				float fullWidth = m_actualSize.x - 2.0f;
				if (m_cursorPosCache > fullWidth)
				{
					m_proxy.m_offset += m_cursorPosCache - fullWidth;
					m_cursorPosCache = fullWidth;
				}
			}
			break;
		}
		case KeyboardEvent::KeyPress:
		{
			switch (code)
			{
				case KeyCode::Left:
				{
					if (m_cursorIndex != 0)
					{
						char const* end = &m_proxy.m_text[m_cursorIndex];
						uint32_t length = 1;
						while ((*--end & 0xc0) == 0x80)
							length++;
						m_cursorIndex -= length;
						Utf8Code code = StringUtils::CodeOfUtf8(m_proxy.m_text.c_str() + m_cursorIndex);
						BitmapGlyph const* glyph = m_proxy.m_font->GetGlyph(code.code);
						if (glyph != nullptr)
						{
							m_cursorPosCache -= glyph->xadvance * m_proxy.m_textSize;
							if (m_cursorPosCache < 0.0f)
							{
								m_proxy.m_offset += m_cursorPosCache;
								m_cursorPosCache = 0.0f;
							}
						}
					}
					break;
				}
				case KeyCode::Right:
				{
					if (m_cursorIndex != m_proxy.m_text.length())
					{
						Utf8Code code = StringUtils::CodeOfUtf8(m_proxy.m_text.c_str() + m_cursorIndex);
						BitmapGlyph const* glyph = m_proxy.m_font->GetGlyph(code.code);
						if (glyph != nullptr)
						{
							m_cursorPosCache += glyph->xadvance * m_proxy.m_textSize;
							float fullWidth = m_actualSize.x - 2.0f;
							if (m_cursorPosCache > fullWidth)
							{
								m_proxy.m_offset += m_cursorPosCache - fullWidth;
								m_cursorPosCache = fullWidth;
							}
						}
						m_cursorIndex += code.length;
					}
					break;
				}
				case KeyCode::Delete:
				{
					if (m_cursorIndex != m_proxy.m_text.length())
					{
						Utf8Code code = StringUtils::CodeOfUtf8(m_proxy.m_text.c_str() + m_cursorIndex);
						m_proxy.m_text.erase(m_proxy.m_text.begin() + m_cursorIndex, m_proxy.m_text.begin() + m_cursorIndex + code.length);
					}
					break;
				}
				case KeyCode::Backspace:
				{
					// Backspace is just Left+Delete.
					if (m_cursorIndex != 0)
					{
						char const* end = &m_proxy.m_text[m_cursorIndex];
						uint32_t length = 1;
						while ((*--end & 0xc0) == 0x80)
							length++;
						m_cursorIndex -= length;
						Utf8Code code = StringUtils::CodeOfUtf8(m_proxy.m_text.c_str() + m_cursorIndex);
						BitmapGlyph const* glyph = m_proxy.m_font->GetGlyph(code.code);
						if (glyph != nullptr)
						{
							m_cursorPosCache -= glyph->xadvance * m_proxy.m_textSize;
							if (m_cursorPosCache < 0.0f)
							{
								m_proxy.m_offset += m_cursorPosCache;
								m_cursorPosCache = 0.0f;
							}
						}
					}
					if (m_cursorIndex != m_proxy.m_text.length())
					{
						Utf8Code code = StringUtils::CodeOfUtf8(m_proxy.m_text.c_str() + m_cursorIndex);
						m_proxy.m_text.erase(m_proxy.m_text.begin() + m_cursorIndex, m_proxy.m_text.begin() + m_cursorIndex + code.length);
					}
					break;
				}
			}
			break;
		}
	}
}

void TextBox::SetOnValueChanged(Function<void()> fn)
{
	m_onValueChanged = std::move(fn);
	m_acceptFocusEvent = m_onValueChanged != nullptr;
}

void TextBox::SetText(String text)
{
	m_proxy.m_text = std::move(text);
	m_proxy.m_offset = 0.0f;
	m_cursorIndex = 0;
	m_cursorPosCache = 0.0f;
}