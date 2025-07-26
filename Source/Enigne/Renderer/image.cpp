#include "Engine/Renderer/image.h"
#include "Engine/Renderer/renderer.h"
using namespace glex;

Image::Image(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube) : m_usages(usages), m_size(size), m_samples(samples), m_cubeMapCompatible(usedAsCube)
{
	m_format = gl::VulkanEnum::FindSuitableImageFormat(format, usages);
	m_imageMemory = m_imageObject.Create(m_format, usages, size, samples, usedAsCube);
	if (m_imageMemory.GetHandle() != VK_NULL_HANDLE)
		m_currentLayout.resize(size.z, gl::ImageLayout::Undefined);
}

Image::~Image()
{
	if (IsValid())
		Renderer::PendingDelete([img = m_imageObject, mem = m_imageMemory]() mutable { img.Destroy(mem); });
}

bool Image::Resize(glm::uvec2 size)
{
	GLEX_ASSERT(IsValid()) {}
	gl::Image newImage;
	gl::Memory newMemory = newImage.Create(m_format, m_usages, glm::uvec3(size, m_size.z), m_samples, m_cubeMapCompatible);
	if (newMemory.GetHandle() != VK_NULL_HANDLE)
	{
		Renderer::PendingDelete([img = m_imageObject, mem = m_imageMemory]() mutable { img.Destroy(mem); });
		m_size.x = size.x;
		m_size.y = size.y;
		m_imageObject = newImage;
		m_imageMemory = newMemory;
		for (gl::ImageLayout& layout : m_currentLayout)
			layout = gl::ImageLayout::Undefined;
		return true;
	}
	Logger::Error("Image resize failed but you can still use the old one.");
	return false;
}

ImageView::ImageView(SharedPtr<Image> const& image, uint32_t layerIndex, uint32_t numLayers, gl::ImageType type, gl::ImageAspect aspect)
	: m_layerIndex(layerIndex), m_numLayers(numLayers), m_type(type), m_aspect(aspect)
{
	if (image->IsValid())
	{
		m_image = image;
		m_imageViewObject.Create(image->GetImageObject(), layerIndex, numLayers, image->Format(), type, aspect);
	}
}

bool ImageView::Recreate()
{
	GLEX_ASSERT(IsValid()) {}
	Renderer::PendingDelete([view = m_imageViewObject]() mutable { view.Destroy(); });
	if (m_imageViewObject.Create(m_image->GetImageObject(), m_layerIndex, m_numLayers, m_image->Format(), m_type, m_aspect))
		return true;
	Logger::Error("Image view recreation failed. Cannot continue.");
	return false;
	// Keep the image reference even on failure.
}

ImageView::~ImageView()
{
	if (IsValid())
		Renderer::PendingDelete([view = m_imageViewObject]() mutable { view.Destroy(); });
}