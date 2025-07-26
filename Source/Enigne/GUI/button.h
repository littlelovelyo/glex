#pragma once
#include "Engine/GUI/text_block.h"
#include "Engine/GUI/control.h"
#include "Core/Container/function.h"

namespace glex::ui
{
	class Button : public ContentControl
	{
	protected:
		Function<void(glm::vec2)> m_click;
		bool m_hovered = false;

	public:
		Button();
		void SetFont(SharedPtr<BitmapFont> const& font) { reinterpret_cast<UniquePtr<TextBlock>&>(m_child)->SetFont(font); }
		void SetText(char const* text) { reinterpret_cast<UniquePtr<TextBlock>&>(m_child)->SetText(text); }
		void SetTextSize(float size) { reinterpret_cast<UniquePtr<TextBlock>&>(m_child)->SetTextSize(size); }
		void SetTextMaterial(SharedPtr<MaterialInstance> const& material) { m_child->SetMaterial(material); }
		void SetTextHorizontalAlignment(HorizontalAlignment horizontalAlignment) { m_child->SetHorizontalAlignment(horizontalAlignment); }
		void SetTextVerticalAlignment(VerticalAlignment verticalAlignment) { m_child->SetVerticalAlignment(verticalAlignment); }
		void SetPadding(glm::vec4 const& padding) { m_child->SetMargin(padding); }
		void SetClick(Function<void(glm::vec2)> click) { m_click = std::move(click); }
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 pos) override;
	};
}