#pragma once
#include "Engine/engine.h"
#include "Engine/ECS/scene.h"
#include "Engine/GUI/control.h"

namespace glex
{
	class GameInstance
	{
	private:
		inline static SharedPtr<Scene> s_currentScene;
		inline static GameInstance* s_instance;
		inline static Vector<UniquePtr<ui::Control>> s_controlStack;
		inline static Vector<std::pair<WeakPtr<ui::Control>, glm::vec2>> s_popupStack;

	public:
		static void Shutdown();
		static void BaseResize(uint32_t width, uint32_t height);
		static GameInstance& Get() { return *s_instance; }
		static void SetCurrentScene(SharedPtr<Scene> const& scene) { s_currentScene = scene; }
		static SharedPtr<Scene> const& GetCurrentScene() { return s_currentScene; }
		static WeakPtr<ui::Control> PushControl(UniquePtr<ui::Control>&& control);
		static UniquePtr<ui::Control> PopControl(WeakPtr<ui::Control> control);
		static void ClearControls();
		static void PushPopup(WeakPtr<ui::Control> popup, glm::vec2 pos);
		static void RemovePopup(WeakPtr<ui::Control> popup);
		static void ClearPopup();
		static void ClearPopupExcept(WeakPtr<ui::Control> popup);
		static Vector<UniquePtr<ui::Control>> const& GetControlStack() { return s_controlStack; }
		static Vector<std::pair<WeakPtr<ui::Control>, glm::vec2>> const& GetPopupStack() { return s_popupStack; }

		GameInstance() { s_instance = this; }
		virtual void Preinitialize(EngineStartupInfo& outInfo) {}
		virtual void BeginPlay() {}
		virtual void Tick() {}
		virtual void EndPlay() {}
		virtual void Resize(uint32_t width, uint32_t height) {}
	};
}