/**
 * Memory aliasing:
 * We don't alias image memory for now (which will be too complicated),
 * but we do want to alias image views.
 */
#pragma once
#include "Core/GL/image.h"
#include "Core/GL/enums.h"
#include "Core/Container/basic.h"
#include "Core/Memory/smart_ptr.h"

namespace glex
{
	class Image : private Unmoveable
	{
	private:
		gl::Image m_imageObject;
		gl::Memory m_imageMemory;
		gl::ImageFormat m_format;
		gl::ImageUsage m_usages;
		uint8_t m_samples;
		bool m_cubeMapCompatible;
		Vector<gl::ImageLayout> m_currentLayout;
		glm::uvec3 m_size;

	public:
		Image(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube = false);
		~Image();
		bool IsValid() const { return m_imageMemory.GetHandle() != VK_NULL_HANDLE; }
		gl::Image GetImageObject() const { return m_imageObject; }
		gl::ImageFormat Format() const { return m_format; }
		uint8_t SampleCount() const { return m_samples; }
		gl::ImageLayout GetImageLayout(uint32_t layer) { return m_currentLayout[layer]; }
		void SetImageLayout(uint32_t layer, uint32_t numLayers, gl::ImageLayout layout) { for (uint32_t i = 0; i < numLayers; i++) m_currentLayout[layer + i] = layout; };
		glm::uvec3 Size() const { return m_size; }
		bool Resize(glm::uvec2 size);
	};

	class ImageView : private Unmoveable
	{
	private:
		SharedPtr<Image> m_image;
		gl::ImageView m_imageViewObject;
		uint32_t m_layerIndex, m_numLayers;
		gl::ImageType m_type;
		gl::ImageAspect m_aspect;
		gl::ImageLayout m_currentLayout;

	public:
		ImageView(SharedPtr<Image> const& image, uint32_t layerIndex, uint32_t numLayers, gl::ImageType type, gl::ImageAspect aspect);
		~ImageView();
		bool IsValid() const { return m_imageViewObject.GetHandle() != VK_NULL_HANDLE; }
		bool Recreate();
		WeakPtr<Image> GetImage() { return m_image; }
		WeakPtr<Image const> GetImage() const { return m_image; }
		gl::ImageView GetImageViewObject() const { return m_imageViewObject; }
		gl::ImageType Type() const { return m_type; }
		gl::ImageAspect Aspect() const { return m_aspect; }
		uint32_t LayerIndex() const { return m_layerIndex; }
		uint32_t LayerCount() const { return m_numLayers; }
	};
}