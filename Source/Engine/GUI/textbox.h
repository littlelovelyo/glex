#pragma once
#include "Engine/GUI/control.h"
#include "Engine/GUI/bitmap_font.h"
#include "Core/Container/function.h"

namespace glex::ui
{
	class TextBoxRenderProxy : public Control
	{
		friend class TextBox;

	private:
		SharedPtr<ui::BitmapFont> m_font;
		String m_text;
		float m_textSize = 16.0f;
		float m_offset = 0.0f;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
	};

	// This class becomes large.
	// But SWidget in Unreal Engine 5 is 672 bytes.
	class TextBox : public Control
	{
	private:
		TextBoxRenderProxy m_proxy;
		uint32_t m_cursorIndex = 0;
		float m_cursorPosCache = 0.0f;
		Function<void()> m_onValueChanged;

	public:
		TextBox(SharedPtr<BitmapFont> const& font, float size);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 pos) override;
		virtual void OnInput(KeyboardEvent event, uint32_t code) override;
		virtual uint32_t NumChildren() const override { return 1; }
		virtual WeakPtr<Control> GetChild(uint32_t index) { return &m_proxy; }
		void SetTextMaterial(SharedPtr<MaterialInstance> const& material) { m_proxy.SetMaterial(material); }
		String const& GetText() const { return m_proxy.m_text; }
		void SetText(String text);
		void SetOnValueChanged(Function<void()> fn);
	};
}