#include "Engine/Renderer/texture.h"
#include "Engine/Renderer/renderer.h"
#include "Core/Utils/raii.h"
#include <stb/stb_image.h>

using namespace glex;

Texture::Texture(char const* imageFile, gl::Sampler sampler)
{
	if (sampler.GetHandle() == VK_NULL_HANDLE)
		return;

	int32_t x, y, channels;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* data = stbi_load(imageFile, &x, &y, &channels, 0);
	if (data == nullptr)
	{
		Logger::Error("Cannot load image file: %s.", imageFile);
		return;
	}
	AutoCleaner cleanBuffer([=]() { stbi_image_free(data); });

	gl::ImageUsage usage = gl::ImageUsage::SampledTexture | gl::ImageUsage::TransferDest;
	gl::ImageFormat format;
	switch (channels)
	{
		case 1: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::R, usage); break;
		case 2: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RG, usage); break;
		case 3: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RGB, usage); break;
		case 4: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RGBA, usage); break;
		default: Logger::Error("%d-channel images are not supported.", channels); return;
	}
	SharedPtr<Image> image = MakeShared<Image>(format, usage, glm::uvec3(x, y, 1), 1);
	if (!image->IsValid())
	{
		Logger::Error("Cannot create image object.");
		return;
	}
	if (!Renderer::UploadImage(image, 0, { x, y }, channels, data))
	{
		Logger::Error("Cannot upload image.");
		return;
	}
	m_imageView.Emplace(image, 0, 1, gl::ImageType::Sampler2D, gl::ImageAspect::Color);
	if (!m_imageView->IsValid())
	{
		Logger::Error("Cannot create image view object.");
		return;
	}
	m_samplerObject = sampler;
}

Texture::Texture(char const* imageFile, gl::ImageFormat formatOverride, gl::Sampler sampler)
{
	if (sampler.GetHandle() == VK_NULL_HANDLE)
		return;

	int32_t desiredChannels;
	int32_t x, y, channels;
	stbi_set_flip_vertically_on_load(true);
	switch (formatOverride)
	{
		case gl::ImageFormat::R: desiredChannels = 1; break;
		case gl::ImageFormat::RG: desiredChannels = 2; break;
		case gl::ImageFormat::RGB: desiredChannels = 3; break;
		case gl::ImageFormat::RGBA: desiredChannels = 4; break;
		default: Logger::Error("Image format %d is not supported.", *formatOverride); return;
	}
	uint8_t* data = stbi_load(imageFile, &x, &y, &channels, desiredChannels);
	if (data == nullptr)
	{
		Logger::Error("Cannot load image file: %s.", imageFile);
		return;
	}
	AutoCleaner cleanBuffer([=]() { stbi_image_free(data); });

	gl::ImageUsage usage = gl::ImageUsage::SampledTexture | gl::ImageUsage::TransferDest;
	gl::ImageFormat format = gl::VulkanEnum::FindSuitableImageFormat(formatOverride, usage);
	SharedPtr<Image> image = MakeShared<Image>(format, usage, glm::uvec3(x, y, 1), 1);
	if (!image->IsValid())
	{
		Logger::Error("Cannot create image object.");
		return;
	}
	if (!Renderer::UploadImage(image, 0, { x, y }, desiredChannels, data))
	{
		Logger::Error("Cannot upload image.");
		return;
	}
	m_imageView.Emplace(image, 0, 1, gl::ImageType::Sampler2D, gl::ImageAspect::Color);
	if (!m_imageView->IsValid())
	{
		Logger::Error("Cannot create image view object.");
		return;
	}
	m_samplerObject = sampler;
}

Texture::Texture(char const* left, char const* right, char const* up, char const* bottom, char const* front, char const* back, gl::Sampler sampler)
{
	int32_t x, y, channels;
	stbi_set_flip_vertically_on_load(false);
	uint8_t* data = stbi_load(right, &x, &y, &channels, 0);
	if (data == nullptr)
	{
		Logger::Error("Cannot load image file: %s.", right);
		return;
	}
	AutoCleaner cleanBuffer([&]() { stbi_image_free(data); });

	gl::ImageUsage usage = gl::ImageUsage::SampledTexture | gl::ImageUsage::TransferDest;
	gl::ImageFormat format;
	switch (channels)
	{
		case 1: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::R, usage); break;
		case 2: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RG, usage); break;
		case 3: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RGB, usage); break;
		case 4: format = gl::VulkanEnum::FindSuitableImageFormat(gl::ImageFormat::RGBA, usage); break;
		default: Logger::Error("%d-channel images are not supported.", channels); return;
	}
	SharedPtr<Image> image = MakeShared<Image>(format, usage, glm::uvec3(x, y, 6), 1, true);
	if (!image->IsValid())
	{
		Logger::Error("Cannot create image object.");
		return;
	}
	if (!Renderer::UploadImage(image, 0, { x, y }, channels, data))
	{
		Logger::Error("Cannot upload image.");
		return;
	}

	auto LoadSubsequent = [&](uint32_t layer, char const* file) -> bool
	{
		stbi_image_free(data);
		int32_t nx, ny, nc;
		data = stbi_load(file, &nx, &ny, &nc, 0);
		if (data == nullptr)
		{
			Logger::Error("Cannot load image file: %s", file);
			return false;
		}
		if (nx != x || ny != y || nc != channels)
		{
			Logger::Error("Size of image %s doesn't match.", file);
			return false;
		}
		if (!Renderer::UploadImage(image, layer, { x, y }, channels, data))
		{
			Logger::Error("Cannot upload image.");
			return false;
		}
		return true;
	};
	if (!LoadSubsequent(1, left) || !LoadSubsequent(2, up) || !LoadSubsequent(3, bottom) || !LoadSubsequent(4, front) || !LoadSubsequent(5, back))
		return;
	m_imageView.Emplace(image, 0, 6, gl::ImageType::SamplerCube, gl::ImageAspect::Color);
	if (!m_imageView->IsValid())
	{
		Logger::Error("Cannot create image view object.");
		return;
	}
	m_samplerObject = sampler;
}

Texture::~Texture()
{
	if (m_samplerObject.GetHandle() != VK_NULL_HANDLE)
		m_imageView.Destroy();
}

void Texture::SetSampler(gl::Sampler sampler)
{
	GLEX_DEBUG_ASSERT(IsValid()) {}
	m_samplerObject = sampler;
}