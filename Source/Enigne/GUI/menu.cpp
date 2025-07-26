#include "Engine/GUI/menu.h"
#include "Core/Platform/input.h"
#include "game.h"

using namespace glex;
using namespace glex::ui;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Menu item.
————————————————————————————————————————————————————————————————————————————————————————————————————*/
MenuItem::MenuItem()
{
	SetHorizontalAlignment(HorizontalAlignment::Stretch);
	SetPadding(glm::vec4(8.0f, 2.0f, 8.0f, 2.0f));
}

UniquePtr<ContextMenu> MenuItem::SetClick(Function<void(glm::vec2 mousePos)> click)
{
	UniquePtr<ContextMenu> result = nullptr;
	if (m_submenu != nullptr)
	{
		GameInstance::RemovePopup(m_submenu);
		result = std::move(m_submenu);
		m_submenu = nullptr;
	}
	Button::SetClick(std::move(click));
	return result;
}

UniquePtr<ContextMenu> MenuItem::SetPopupMenu(UniquePtr<ContextMenu>&& submenu)
{
	UniquePtr<ContextMenu> result = std::move(m_submenu);
	m_submenu = std::move(submenu);
	Button::SetClick([this](glm::vec2 mousePos) { GameInstance::PushPopup(m_submenu, glm::vec2(Input::MouseX(), Input::MouseY())); });
	return result;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Pop-up menu.
————————————————————————————————————————————————————————————————————————————————————————————————————*/
ContextMenu::ContextMenu(SharedPtr<MaterialInstance> const& material)
{
	SetMaterial(material);
}

WeakPtr<MenuItem> ContextMenu::AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, Function<void(glm::vec2)> click)
{
	UniquePtr<MenuItem> menuItem = MakeUnique<MenuItem>();
	menuItem->SetMaterial(m_material);
	menuItem->SetTextMaterial(textMaterial);
	menuItem->SetFont(font);
	menuItem->SetText(caption);
	menuItem->SetClick(std::move(click));
	WeakPtr<MenuItem> ret = menuItem;
	PushChild(std::move(menuItem));
	return ret;
}

WeakPtr<MenuItem> ContextMenu::AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, UniquePtr<ContextMenu>&& submenu)
{
	UniquePtr<MenuItem> menuItem = MakeUnique<MenuItem>();
	menuItem->SetMaterial(m_material);
	menuItem->SetTextMaterial(textMaterial);
	menuItem->SetFont(font);
	menuItem->SetText(caption);
	menuItem->SetPopupMenu(std::move(submenu));
	WeakPtr<MenuItem> ret = menuItem;
	PushChild(std::move(menuItem));
	return menuItem;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Menu bar.
————————————————————————————————————————————————————————————————————————————————————————————————————*/
Menu::Menu(SharedPtr<MaterialInstance> const& material)
{
	SetMaterial(material);
}

WeakPtr<MenuItem> Menu::AddMenu(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, UniquePtr<ContextMenu>&& popupMenu)
{
	UniquePtr<MenuItem> menuItem = MakeUnique<MenuItem>();
	menuItem->SetMaterial(m_material);
	menuItem->SetTextMaterial(textMaterial);
	menuItem->SetFont(font);
	menuItem->SetText(caption);
	menuItem->SetPopupMenu(std::move(popupMenu));
	WeakPtr<MenuItem> ret = menuItem;
	PushChild(std::move(menuItem));
	return ret;
}

WeakPtr<MenuItem> Menu::AddMenuItem(char const* caption, SharedPtr<MaterialInstance> const& textMaterial, SharedPtr<BitmapFont> const& font, Function<void(glm::vec2)> click)
{
	UniquePtr<MenuItem> menuItem = MakeUnique<MenuItem>();
	menuItem->SetMaterial(m_material);
	menuItem->SetTextMaterial(textMaterial);
	menuItem->SetFont(font);
	menuItem->SetText(caption);
	menuItem->SetClick(std::move(click));
	WeakPtr<MenuItem> ret = menuItem;
	PushChild(std::move(menuItem));
	return ret;
}