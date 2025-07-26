#pragma once
#include "Core/GL/buffer.h"
#include "Core/GL/enums.h"

namespace glex::gl
{
	class Image
	{
	private:
		VkImage m_handle;

	public:
		Image() : m_handle(VK_NULL_HANDLE) {}
		Image(VkImage handle) : m_handle(handle) {}
		Memory Create(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube = false);
		void Destroy(Memory memory);
		VkImage GetHandle() const { return m_handle; }
	};

	class ImageView
	{
	private:
		VkImageView m_handle;

	public:
		ImageView() : m_handle(VK_NULL_HANDLE) {}
		ImageView(VkImageView handle) : m_handle(handle) {}
		bool Create(Image image, uint32_t layer, uint32_t numLayers, ImageFormat format, ImageType type, ImageAspect aspect);
		void Destroy();
		VkImageView GetHandle() const { return m_handle; }
	};

	class Sampler
	{
	private:
		VkSampler m_handle;

	public:
		Sampler() : m_handle(VK_NULL_HANDLE) {}
		Sampler(VkSampler handle) : m_handle(handle) {}
		bool Create(ImageFilter magFilter, ImageFilter minFilter, ImageWrap wrapU, ImageWrap wrapV, ImageWrap wrapW, float anisotropyLevel);
		void Destroy();
		VkSampler GetHandle() const { return m_handle; }
	};
}