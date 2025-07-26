#include "Engine/GUI/text_block.h"
#include "Engine/GUI/batch.h"
#include "Core/Utils/string.h"

using namespace glex::ui;

// Advance until we hit a space to non-latin character.
char const* TextBlock::FindNextWrappable(char const* p)
{
	for (;;)
	{
		Utf8Code code = StringUtils::CodeOfUtf8(p);
		if (code.code == 0 || code.length == 0)
			return p;
		p += code.length;
		if (code.code == ' ' || !IsLatin(code.code))
			return p;
	}
}

float TextBlock::MeasureWidth(char const* begin, char const* end)
{
	float width = 0.0f;
	while (begin < end)
	{
		Utf8Code code = StringUtils::CodeOfUtf8(begin);
		BitmapGlyph const* glyph = m_font->GetGlyph(code.code);
		if (glyph != nullptr)
			width += glyph->xadvance;
		begin += code.length;
	}
	return width * m_textSize;
}

glm::vec2 TextBlock::OnMeasure(glm::vec2 availableSize)
{
	if (m_font == nullptr || m_text.empty())
		return glm::vec2(0, 0);
	float lineHeight = m_font->LineHeight() * m_textSize;
	// If width is set to 'auto' we don't wrap the text anyway.
	if (m_wrap)
	{
		float width = 0.0f, height = lineHeight, cursor = 0.0f;
		char const* p = m_text.c_str();
		while (*p != 0)
		{
			// Special chars.
			if (*p == '\n')
			{
				cursor = 0.0f;
				height += lineHeight;
				p++;
				continue;
			}

			char const* n = FindNextWrappable(p);
			if (p == n) // Invalid UTF-8 char.
				break;
			float wordWidth = MeasureWidth(p, n);
			// Wrap a word.
			if (cursor != 0.0f && cursor + wordWidth > availableSize.x)
			{
				cursor = 0.0f;
				height += lineHeight;
			}
			cursor += wordWidth;
			if (cursor > width)
				width = cursor;
			p = n;
		}
		return glm::vec2(width, height);
	}
	else
	{
		float width = 0.0f;
		char const* p = m_text.c_str();
		while (*p != 0)
		{
			Utf8Code code = StringUtils::CodeOfUtf8(p);
			if (code.length == 0)
				break;
			BitmapGlyph const* glyph = m_font->GetGlyph(code.code);
			if (glyph != nullptr)
				width += glyph->xadvance;
			p += code.length;
		}
		return glm::vec2(width * m_textSize, lineHeight);
	}
}

void TextBlock::OnPaint(glm::vec2 pos, glm::vec2 size)
{
	if (m_font == nullptr || m_text.empty())
		return;

	if (m_wrap)
	{
		glm::vec2 cursor = pos;
		float lineHeight = m_font->LineHeight() * m_textSize;
		float xend = pos.x + size.x;
		char const* p = m_text.c_str();
		while (*p != 0)
		{
			// Special chars.
			if (*p == '\n')
			{
				cursor.x = pos.x;
				cursor.y += lineHeight;
				p++;
				continue;
			}

			char const* n = FindNextWrappable(p);
			if (p == n)
				break;
			float wordWidth = MeasureWidth(p, n);
			if (cursor.x != 0.0f && cursor.x + wordWidth > xend)
			{
				cursor.x = pos.x;
				cursor.y += lineHeight;
			}
			while (p < n)
			{
				Utf8Code code = StringUtils::CodeOfUtf8(p);
				BitmapGlyph const* glyph = m_font->GetGlyph(code.code);
				if (glyph != nullptr)
				{
					glm::vec2 corner = cursor + glyph->offset * m_textSize;
					BatchRenderer::DrawQuad(glm::vec4(corner, glyph->size * m_textSize),
						m_font->GetTexture(glyph->page), glyph->uv, glm::vec4(m_color, 1.0f));
					cursor.x += glyph->xadvance * m_textSize;
				}
				p += code.length;
			}
		}
	}
	else
	{
		char const* p = m_text.c_str();
		while (*p != 0)
		{
			Utf8Code code = StringUtils::CodeOfUtf8(p);
			if (code.length == 0)
				break;
			BitmapGlyph const* glyph = m_font->GetGlyph(code.code);
			if (glyph != nullptr)
			{
				BatchRenderer::DrawQuad(glm::vec4(pos + glyph->offset * m_textSize, glyph->size * m_textSize),
					m_font->GetTexture(glyph->page), glyph->uv, glm::vec4(m_color, 1.0f));
				pos.x += glyph->xadvance * m_textSize;
			}
			p += code.length;
		}
	}
}