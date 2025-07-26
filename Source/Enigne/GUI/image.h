#pragma once
#include "Engine/Renderer/texture.h"
#include "Engine/GUI/control.h"
#include "Core/assert.h"

namespace glex::ui
{
	class Image : public Control
	{
	private:
		SharedPtr<Texture> m_image;

	public:
		glm::vec2 OnMeasure(glm::vec2 availableSize);
		void OnPaint(glm::vec2 pos, glm::vec2 size);
		void SetImage(SharedPtr<Texture> const& image) { GLEX_ASSERT(image->IsValid()) m_image = image; InvalidateMeasure(); }
	};
}