#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class VerticalBox : public Control
	{
	protected:
		Vector<UniquePtr<Control>> m_children;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override { return m_children.size(); }
		virtual WeakPtr<Control> GetChild(uint32_t index) override { return m_children[index]; };
		void PushChild(UniquePtr<Control> child);
		UniquePtr<Control> PopChild();
		void PopChild(uint32_t count);
		UniquePtr<Control> RemoveChild(uint32_t index);
	};

	class HorizontalBox : public Control
	{
	protected:
		Vector<UniquePtr<Control>> m_children;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override { return m_children.size(); }
		virtual WeakPtr<Control> GetChild(uint32_t index) override { return m_children[index]; };
		void PushChild(UniquePtr<Control> child);
		UniquePtr<Control> PopChild();
		void PopChild(uint32_t count);
		UniquePtr<Control> RemoveChild(uint32_t index);
	};
}