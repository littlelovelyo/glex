#include "game.h"

using namespace glex;
using namespace glex::ui;

void GameInstance::Shutdown()
{
	s_currentScene = nullptr;
	s_controlStack.clear();
	s_popupStack.clear();
#if GLEX_REPORT_MEMORY_LEAKS
	s_controlStack.shrink_to_fit();
	s_popupStack.shrink_to_fit();
#endif
}

void GameInstance::BaseResize(uint32_t width, uint32_t height)
{
	for (UniquePtr<Control>& control : s_controlStack)
		control->Measure(glm::vec2(width, height));
}

WeakPtr<Control> GameInstance::PushControl(UniquePtr<Control>&& control)
{
	WeakPtr<Control> ret = s_controlStack.emplace_back(std::move(control));
	return ret;
}

UniquePtr<Control> GameInstance::PopControl(WeakPtr<Control> control)
{
	auto iter = eastl::find(s_controlStack.begin(), s_controlStack.end(), control, [](UniquePtr<Control> const& elem, WeakPtr<Control> item)
	{
		return elem == item;
	});
	if (iter == s_controlStack.end())
		return nullptr;
	UniquePtr<Control> ret = std::move(*iter);
	s_controlStack.erase(iter);
	return ret;
}

void GameInstance::ClearControls()
{
	s_controlStack.clear();
}

void GameInstance::PushPopup(WeakPtr<ui::Control> popup, glm::vec2 pos)
{
	s_popupStack.emplace_back(popup, pos);
}

void GameInstance::RemovePopup(WeakPtr<ui::Control> popup)
{
	auto iter = eastl::find(s_popupStack.begin(), s_popupStack.end(), popup, [](auto const& lhs, WeakPtr<ui::Control> rhs) { return lhs.first == rhs; });
	if (iter != s_popupStack.end())
		s_popupStack.erase(iter);
}

void GameInstance::ClearPopup()
{
	s_popupStack.clear();
}

void GameInstance::ClearPopupExcept(WeakPtr<ui::Control> popup)
{
	if (popup == nullptr)
	{
		s_popupStack.clear();
		return;
	}
	while (popup->GetParent() != nullptr)
		popup = popup->GetParent();
	auto iter = eastl::find(s_popupStack.begin(), s_popupStack.end(), popup, [](auto const& lhs, WeakPtr<ui::Control> rhs) { return lhs.first == rhs; });
	if (iter != s_popupStack.end())
	{
		auto dataCopy = *iter;
		s_popupStack.clear();
		s_popupStack.push_back(dataCopy);
	}
	else
		s_popupStack.clear();
}