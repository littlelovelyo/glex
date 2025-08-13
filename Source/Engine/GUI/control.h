#pragma once
#include "Core/Memory/smart_ptr.h"
#include "Core/Container/basic.h"
#include "Engine/Renderer/matinst.h"
#include <glm/glm.hpp>

namespace glex::ui
{
	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			Enum definition
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	enum class Visibility : uint8_t
	{
		Collapsed,
		Hidden,
		Visible
	};

	enum class HorizontalAlignment : uint8_t
	{
		Left,
		Center,
		Right,
		Stretch
	};

	enum class VerticalAlignment : uint8_t
	{
		Top,
		Center,
		Bottom,
		Stretch
	};

	enum class MouseEvent : uint8_t
	{
		Enter,
		Move,
		Leave,
		Press,
		Release,
		DoubleClick,
		Abort,
		Scroll
	};

	enum class KeyboardEvent : uint8_t
	{
		KeyPress,
		CharInput
	};

	class Control
	{
	protected:
		// Layout.
		WeakPtr<Control> m_parent;
		glm::vec2 m_measureCache = glm::vec2(NAN, NAN);
		glm::vec2 m_arrangeCache = glm::vec2(NAN, NAN);
		glm::vec2 m_desiredSize = glm::vec2(NAN, NAN);
		union
		{
			glm::vec4 m_rect = glm::vec4(NAN, NAN, NAN, NAN); // RELATIVE TO ITS PARENT.
			struct { glm::vec2 m_position, m_actualSize; };
		};
		glm::vec4 m_margin = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		// Rendering.
		SharedPtr<MaterialInstance> m_material;
		int32_t m_zOrder = 0;

		Visibility m_visibility = Visibility::Visible;
		HorizontalAlignment m_horizontalAlignment = HorizontalAlignment::Left;
		VerticalAlignment m_verticalAlignment = VerticalAlignment::Top;
		bool m_acceptMouseEvent : 1 = false;
		bool m_acceptKeyboardEvent : 1 = false;
		bool m_acceptFocusEvent : 1 = false;

		void InvalidateMeasure();

	public:
#if GLEX_INTERNAL
		void SetParent(WeakPtr<Control> parent) { m_parent = parent; }
#endif
		virtual ~Control() = default;
		glm::vec2 Measure(glm::vec2 availableSize);                        // Get desired size. (from child to parent)
		void Arrange(glm::vec2 pos, glm::vec2 actualSize);                 // From parent to child.
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) = 0;          // Override this.
		virtual void OnArrange() {};
		virtual void OnPaint(glm::vec2 position, glm::vec2 size) {};       // Paint itself.
		virtual void OnMouseEvent(MouseEvent event, glm::vec2 pos) {}
		virtual void OnInput(KeyboardEvent event, uint32_t code) {};
		virtual void OnLoseFocus() {};
		virtual uint32_t NumChildren() const { return 0; }
		virtual WeakPtr<Control> GetChild(uint32_t index) { return nullptr; };
		SharedPtr<MaterialInstance> const& GetMaterial() const { return m_material; }
		void SetMaterial(SharedPtr<MaterialInstance> const& material) { m_material = material; }
		WeakPtr<Control> GetParent() const { return m_parent; }
		int32_t GetZOrder() const { return m_zOrder; }
		void SetZOrder(int32_t zOrder) { m_zOrder = zOrder; }
		Visibility GetVisibility() const { return m_visibility; }
		glm::vec4 const& GetRect() const { return m_rect; }
		glm::vec2 const& GetDesiredSize() const { return m_desiredSize; }
		glm::vec4 const& GetMargin() const { return m_margin; }
		HorizontalAlignment GetHorizontalAlignment() const { return m_horizontalAlignment; }
		VerticalAlignment GetVerticalAlignment() const { return m_verticalAlignment; }
		bool GetAcceptMouseEvent() const { return m_acceptMouseEvent; }
		bool GetAcceptKeyboardEvent() const { return m_acceptKeyboardEvent; }
		bool GetAcceptFocusEvent() const { return m_acceptFocusEvent; }
		void SetHorizontalAlignment(HorizontalAlignment horizontalAlignment);
		void SetVerticalAlignment(VerticalAlignment verticalAlignment);
		void SetVisibility(Visibility visibility);
		void SetMargin(glm::vec4 const& margin);
	};

	class ContentControl : public Control
	{
	protected:
		UniquePtr<Control> m_child;

	public:
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const override { return m_child != nullptr ? 1 : 0; }
		virtual WeakPtr<Control> GetChild(uint32_t index) override { return m_child; }
		UniquePtr<Control> SetChild(UniquePtr<Control> newChild);
	};
}