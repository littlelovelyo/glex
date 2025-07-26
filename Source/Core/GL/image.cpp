#include "Core/GL/image.h"
#include "Core/GL/context.h"
#include "Core/GL/enums.h"
#include <vma/vk_mem_alloc.h>

using namespace glex::gl;

Memory Image::Create(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	if (usedAsCube)
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VulkanEnum::GetImageFormat(format);
	imageInfo.extent = { size.x, size.y, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = size.z;
	imageInfo.samples = static_cast<VkSampleCountFlagBits>(samples);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VulkanEnum::GetImageUsage(usages);
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	VmaAllocation allocation;
	if (vmaCreateImage(Context::GetAllocator(), &imageInfo, &allocationInfo, &m_handle, &allocation, nullptr) == VK_SUCCESS)
		return Memory(allocation);
	m_handle = VK_NULL_HANDLE;
	return Memory(VK_NULL_HANDLE);
}

void Image::Destroy(Memory memory)
{
	vmaDestroyImage(Context::GetAllocator(), m_handle, memory.GetHandle());
}

bool ImageView::Create(Image image, uint32_t layer, uint32_t numLayers, ImageFormat format, ImageType type, ImageAspect aspect)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.GetHandle();
	viewInfo.viewType = VulkanEnum::GetImageType(type);
	viewInfo.format = VulkanEnum::GetImageFormat(format);
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VulkanEnum::GetImageAspect(aspect);
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = layer;
	viewInfo.subresourceRange.layerCount = numLayers;
	if (vkCreateImageView(Context::GetDevice(), &viewInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void ImageView::Destroy()
{
	vkDestroyImageView(Context::GetDevice(), m_handle, Context::HostAllocator());
}

bool Sampler::Create(ImageFilter magFilter, ImageFilter minFilter, ImageWrap wrapU, ImageWrap wrapV, ImageWrap wrapW, float anisotropyLevel)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = static_cast<VkFilter>(magFilter);
	samplerInfo.minFilter = static_cast<VkFilter>(minFilter);
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = static_cast<VkSamplerAddressMode>(wrapU);
	samplerInfo.addressModeV = static_cast<VkSamplerAddressMode>(wrapV);
	samplerInfo.addressModeW = static_cast<VkSamplerAddressMode>(wrapW);
	if (anisotropyLevel != 0.0f)
	{
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = anisotropyLevel;
	}
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	if (vkCreateSampler(Context::GetDevice(), &samplerInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void Sampler::Destroy()
{
	vkDestroySampler(Context::GetDevice(), m_handle, Context::HostAllocator());
}