#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class HorizontalSplitter : public Control
	{
	private:
		UniquePtr<Control> m_left;
		UniquePtr<Control> m_right;
		float m_splitterPosition = 0.0f;

	public:
		HorizontalSplitter();
		UniquePtr<Control> SetLeft(UniquePtr<Control>&& left);
		UniquePtr<Control> SetRight(UniquePtr<Control>&& right);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override;
		virtual WeakPtr<Control> GetChild(uint32_t index) override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 mousePos) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
	};

	class VerticalSplitter : public Control
	{
	private:
		UniquePtr<Control> m_upper;
		UniquePtr<Control> m_lower;
		float m_splitterPosition = 0.0f;

	public:
		VerticalSplitter();
		UniquePtr<Control> SetUpper(UniquePtr<Control>&& upper);
		UniquePtr<Control> SetLower(UniquePtr<Control>&& lower);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override;
		virtual WeakPtr<Control> GetChild(uint32_t index) override;
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 mousePos) override;
		virtual void OnPaint(glm::vec2 pos, glm::vec2 size) override;
	};
}