#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	// Upper uses auto size, lower takes up all the rest space.
	class VerticalTwoSlices : public Control
	{
	private:
		UniquePtr<Control> m_upper;
		UniquePtr<Control> m_lower;

	public:
		VerticalTwoSlices();
		UniquePtr<Control> SetUpper(UniquePtr<Control> control);
		UniquePtr<Control> SetLower(UniquePtr<Control> control);
		WeakPtr<Control> GetUpper() { return m_upper; }
		WeakPtr<Control> GetLower() { return m_lower; }
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override;
		virtual WeakPtr<Control> GetChild(uint32_t index) override;
	};

	// Upper uses auto size, lower takes up all the rest space.
	class HorizontalTwoSlices : public Control
	{
	private:
		UniquePtr<Control> m_left;
		UniquePtr<Control> m_right;

	public:
		HorizontalTwoSlices();
		UniquePtr<Control> SetLeft(UniquePtr<Control> control);
		UniquePtr<Control> SetRight(UniquePtr<Control> control);
		WeakPtr<Control> GetLeft() { return m_left; }
		WeakPtr<Control> GetRight() { return m_right; }
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override;
		virtual WeakPtr<Control> GetChild(uint32_t index) override;
	};
}