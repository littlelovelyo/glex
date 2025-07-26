#include "Engine/Renderer/pipeline.h"
#include "Engine/Renderer/renderer.h"
#include "Core/Utils/raii.h"
#include "Core/GL/context.h"

using namespace glex;

bool Pipeline::BindGlobalData(SequenceView<ShaderResource const> resources)
{
	uint32_t uniformBufferCount = 0;
	uint32_t textureCount = 0;

	Vector<gl::DescriptorBinding> descriptorLayout(resources.Size());
	Vector<gl::Descriptor> descriptors(resources.Size());
	Vector<gl::BufferDescriptor> buffers;
	Vector<gl::ImageSamplerDesciptor> textures;

	for (uint32_t i = 0; i < resources.Size(); i++)
	{
		ShaderResource const& resource = resources[i];
		gl::DescriptorBinding& binding = descriptorLayout[i];
		gl::Descriptor& descriptor = descriptors[i];
		binding.bindingPoint = i;
		binding.arraySize = 1; // TODO: texture array.
		binding.type = resource.type;
		binding.shaderStage = gl::ShaderStage::AllGraphics; // Why not?
		descriptor.bindingPoint = i;
		descriptor.arrayIndex = 0;
		descriptor.type = resource.type;

		if (resource.type == gl::DescriptorType::UniformBuffer)
		{
			uniformBufferCount++;
			if (!resource.buffer->IsValid())
				return false;
			gl::BufferDescriptor& buffer = buffers.emplace_back();
			buffer.buffer = resource.buffer->GetBufferObject();
			buffer.offset = 0;
			buffer.size = resource.buffer->Size();
			descriptor.buffers = &buffer;
		}
		else
		{
			textureCount++;
			if (!resource.texture->IsValid())
				return false;
			gl::ImageSamplerDesciptor& texture = textures.emplace_back();
			texture.image.imageView = resource.texture->GetImageView().GetImageViewObject();
			texture.image.imageLayout = gl::ImageLayout::ShaderRead;
			texture.sampler.sampler = resource.texture->GetSampler();
			descriptor.imageSamplers = &texture;
		}
	}

	std::pair<gl::DescriptorType, uint32_t> descriptorCounts[2];
	uint32_t numTypes = 0;
	if (uniformBufferCount != 0)
	{
		descriptorCounts[numTypes] = { gl::DescriptorType::UniformBuffer, uniformBufferCount };
		numTypes++;
	}
	if (textureCount != 0)
	{
		descriptorCounts[numTypes] = { gl::DescriptorType::CombinedImageSampler, textureCount };
		numTypes++;
	}

	if (!m_globalDescriptorPool.Create(SequenceView(descriptorCounts, numTypes), 1, false))
		return false;
	AutoCleaner poolCleaner([this]()
	{
		if (m_globalDescriptorSet.GetHandle() == VK_NULL_HANDLE)
			m_globalDescriptorPool.Destroy();
	});

	m_globalDescriptorSetLayout = Renderer::GetDescriptorLayoutCache().GetDescriptorSetLayout(descriptorLayout);
	if (m_globalDescriptorSetLayout.GetHandle() == VK_NULL_HANDLE)
		return false;
	AutoCleaner layoutCleaner([this]()
	{
		if (m_globalDescriptorSet.GetHandle() == VK_NULL_HANDLE)
			Renderer::GetDescriptorLayoutCache().FreeDescriptorSetLayout(m_globalDescriptorSetLayout);
	});

	// No way this can fail, right?
	m_globalDescriptorSet = m_globalDescriptorPool.AllocateDescriptorSet(m_globalDescriptorSetLayout);
	if (m_globalDescriptorSet.GetHandle() == VK_NULL_HANDLE)
		return false;
	m_globalDescriptorSet.BindDescriptors(descriptors);
	return true;
}

void Pipeline::BaseShutdown()
{
	if (m_globalDescriptorSet.GetHandle() != VK_NULL_HANDLE)
	{
		m_globalDescriptorPool.Reset();
		m_globalDescriptorPool.Destroy();
		Renderer::GetDescriptorLayoutCache().FreeDescriptorSetLayout(m_globalDescriptorSetLayout);
	}
}

void Pipeline::SetGlobalData(WeakPtr<Buffer> buffer, void const* data, uint32_t size)
{
	Renderer::UploadBufferDynamic(buffer, 0, size, data, gl::PipelineStage::None, gl::Access::None, gl::PipelineStage::VertexShader, gl::Access::UniformRead);
}