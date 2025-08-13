#pragma once
#include "Engine/GUI/control.h"
#include "Engine/GUI/text_block.h"
#include "Core/Container/basic.h"
#include "Core/Container/function.h"

namespace glex::ui
{
	class ListBox : public Control
	{
	private:
		SharedPtr<BitmapFont> m_font;
		Vector<UniquePtr<TextBlock>> m_items;
		uint32_t m_hoverIndex = -1;
		uint32_t m_selectIndex = -1;
		float m_offset = 0.0f;
		float m_textSize = 16.0f;
		Function<void(uint32_t)> m_onDoubleClick;

		void CalculateStuff();

	public:
		ListBox(SharedPtr<MaterialInstance> const& material, SharedPtr<BitmapFont> const& font);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override;
		virtual WeakPtr<Control> GetChild(uint32_t index) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 pos) override;
		void SetDoubleClick(Function<void(uint32_t)> fn) { m_onDoubleClick = std::move(fn); }
		uint32_t NumItems() const { return m_items.size(); }
		void AddItem(String text, SharedPtr<MaterialInstance> const& textMaterial);
		void RemoveItem(uint32_t index);
		uint32_t GetSelection() const { return m_selectIndex; }
		void SetSelection(uint32_t index);
		String const& GetItemText(uint32_t index);
		void Clear();
	};
}