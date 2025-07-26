#pragma once
#include "Engine/GUI/text_block.h"
#include "Core/Container/function.h"

namespace glex::ui
{
	class TreeView;
	class TreeViewItem : public Control
	{
		friend class TreeView;

	private:
		TextBlock m_header;
		Vector<UniquePtr<TreeViewItem>> m_children;
		WeakPtr<TreeView> m_owner;
		void* m_userData;
		bool m_expanded;
		bool m_hovered;

	public:
		TreeViewItem(WeakPtr<TreeView> owner, SharedPtr<MaterialInstance> const& material, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 mousePos) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
		virtual uint32_t NumChildren() const override { return m_expanded ? m_children.size() + 1 : 1; }
		virtual WeakPtr<Control> GetChild(uint32_t index) override { return index == 0 ? &m_header : WeakPtr<Control>(m_children[index - 1]); }
		void SetHeader(String string) { m_header.SetText(std::move(string)); }
		String const& GetHeader() const { return m_header.GetText(); }
		void PushChild(UniquePtr<TreeViewItem> child);
		void RemoveItem(WeakPtr<TreeViewItem> item);
		template <concepts::CanFitInto<8> T> void SetData(T data) { reinterpret_cast<T&>(m_userData) = data; }
		template <concepts::CanFitInto<8> T> T GetData() const { return reinterpret_cast<T const&>(m_userData); }
	};

	class TreeView : public Control
	{
	private:
		Vector<UniquePtr<TreeViewItem>> m_children;
		WeakPtr<TreeViewItem> m_hoverItem;
		WeakPtr<TreeViewItem> m_activeItem;
		Function<bool(WeakPtr<TreeViewItem>, WeakPtr<TreeViewItem>)> m_onSelectionChanged;
		Function<void(WeakPtr<TreeViewItem>, WeakPtr<TreeViewItem>)> m_onDrag;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override { return m_children.size(); }
		virtual WeakPtr<Control> GetChild(uint32_t index) override { return m_children[index]; }
		void Clear();
		void PushChild(UniquePtr<TreeViewItem> child);
		void RemoveItem(WeakPtr<TreeViewItem> item);
		void SetHoverItem(WeakPtr<TreeViewItem> item) { m_hoverItem = item; }
		void SetActiveItem(WeakPtr<TreeViewItem> item);
		WeakPtr<TreeViewItem> GetActiveItem() const { return m_activeItem; }
		void SetOnSelectionChanged(Function<bool(WeakPtr<TreeViewItem>, WeakPtr<TreeViewItem>)> fn) { m_onSelectionChanged = std::move(fn); }
		void SetOnDrag(Function<void(WeakPtr<TreeViewItem>, WeakPtr<TreeViewItem>)> fn) { m_onDrag = std::move(fn); }
		void DispatchDragEvent(WeakPtr<TreeViewItem> from);
	};
}