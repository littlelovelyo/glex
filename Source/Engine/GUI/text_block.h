#pragma once
#include "Engine/GUI/control.h"
#include "Engine/GUI/bitmap_font.h"

namespace glex::ui
{
	class TextBlock : public Control
	{
	protected:
		SharedPtr<BitmapFont> m_font;
		String m_text;
		float m_textSize = 16.0f;
		glm::vec3 m_color = glm::vec3(1.0f, 1.0f, 1.0f);
		bool m_wrap = false;

		bool IsLatin(uint32_t chr) { return chr < 0x80; }
		char const* FindNextWrappable(char const* p);
		float MeasureWidth(char const* begin, char const* end);

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		void SetFont(SharedPtr<BitmapFont> const& font) { m_font = font; InvalidateMeasure(); }
		void SetText(String text) { m_text = std::move(text); InvalidateMeasure(); }
		void SetTextSize(float size) { m_textSize = size; InvalidateMeasure(); }
		void SetWrap(bool wrap) { m_wrap = wrap; InvalidateMeasure(); }
		bool GetWrap() const { return m_wrap; }
		void SetColor(glm::vec3 const& color) { m_color = color; }
		glm::vec3 const& GetColor() const { return m_color; }
		String const& GetText() const { return m_text; }
	};
}