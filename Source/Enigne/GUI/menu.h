#pragma once
#include "Engine/GUI/stack_panel.h"
#include "Engine/GUI/button.h"

namespace glex::ui
{
	class ContextMenu;
	class MenuItem : public Button
	{
	private:
		UniquePtr<ContextMenu> m_submenu;

	public:
		MenuItem();
		UniquePtr<ContextMenu> SetClick(Function<void(glm::vec2)> click);
		UniquePtr<ContextMenu> SetPopupMenu(UniquePtr<ContextMenu>&& submenu);
	};

	class ContextMenu : public VerticalBox
	{
	public:
		ContextMenu(SharedPtr<MaterialInstance> const& material);
		WeakPtr<MenuItem> AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, Function<void(glm::vec2)> click);
		WeakPtr<MenuItem> AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, UniquePtr<ContextMenu>&& submenu);
	};

	class Menu : public HorizontalBox
	{
	public:
		Menu(SharedPtr<MaterialInstance> const& material);
		WeakPtr<MenuItem> AddMenu(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, UniquePtr<ContextMenu>&& popupMenu);
		WeakPtr<MenuItem> AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, Function<void(glm::vec2)> click);
	};
}