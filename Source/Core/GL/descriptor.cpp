#include "Core/GL/descriptor.h"
#include "Core/GL/context.h"

using namespace glex::gl;

bool DescriptorSetLayout::Create(SequenceView<DescriptorBinding const> bindings)
{
	Vector<VkDescriptorSetLayoutBinding> descriptorSetLayout(bindings.Size());
	for (uint32_t i = 0; i < bindings.Size(); i++)
	{
		VkDescriptorSetLayoutBinding& binding = descriptorSetLayout[i];
		DescriptorBinding const& info = bindings[i];
		binding.binding = info.bindingPoint;
		binding.descriptorType = VulkanEnum::GetDescriptorType(info.type);
		binding.descriptorCount = info.arraySize;
		binding.stageFlags = VulkanEnum::GetShaderStage(info.shaderStage);
	}
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.Size();
	layoutInfo.pBindings = descriptorSetLayout.data();
	if (vkCreateDescriptorSetLayout(Context::GetDevice(), &layoutInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void DescriptorSetLayout::Destroy()
{
	vkDestroyDescriptorSetLayout(Context::GetDevice(), m_handle, Context::HostAllocator());
}

bool DescriptorLayout::Create(SequenceView<DescriptorSetLayout const> descriptorSets, ShaderStage constantStages)
{
	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = descriptorSets.Size();
	layoutInfo.pSetLayouts = reinterpret_cast<VkDescriptorSetLayout const*>(descriptorSets.Data());
	if (constantStages != ShaderStage::None)
	{
		VkPushConstantRange constantRange;
		constantRange.stageFlags = VulkanEnum::GetShaderStage(constantStages);
		constantRange.offset = 0;
		constantRange.size = 128; // Use the full size anyway.
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &constantRange;
	}
	if (vkCreatePipelineLayout(Context::GetDevice(), &layoutInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void DescriptorLayout::Destroy()
{
	vkDestroyPipelineLayout(Context::GetDevice(), m_handle, Context::HostAllocator());
}

bool DescriptorPool::Create(SequenceView<std::pair<DescriptorType, uint32_t> const> size, uint32_t maxNumSets, bool freeIndividual)
{
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	if (freeIndividual)
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = maxNumSets;
	poolInfo.poolSizeCount = size.Size();
	poolInfo.pPoolSizes = reinterpret_cast<VkDescriptorPoolSize const*>(size.Data());
	if (vkCreateDescriptorPool(Context::GetDevice(), &poolInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void DescriptorPool::Destroy()
{
	vkDestroyDescriptorPool(Context::GetDevice(), m_handle, Context::HostAllocator());
}

DescriptorSet DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_handle;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = reinterpret_cast<VkDescriptorSetLayout*>(&layout);
	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(Context::GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
		descriptorSet = VK_NULL_HANDLE;
	return DescriptorSet(descriptorSet);
}

void DescriptorPool::FreeDescriptorSet(DescriptorSet descriptorSet)
{
	vkFreeDescriptorSets(Context::GetDevice(), m_handle, 1, reinterpret_cast<VkDescriptorSet*>(&descriptorSet));
}

void DescriptorPool::Reset()
{
	vkResetDescriptorPool(Context::GetDevice(), m_handle, 0);
}

void DescriptorSet::BindDescriptors(SequenceView<Descriptor const> resources)
{
	Vector<VkWriteDescriptorSet> descriptorInfo(resources.Size());
	Vector<Vector<VkDescriptorBufferInfo>> bufferInfoList;
	Vector<Vector<VkDescriptorImageInfo>> imageInfoList;
	for (uint32_t i = 0; i < resources.Size(); i++)
	{
		VkWriteDescriptorSet& writeInfo = descriptorInfo[i];
		Descriptor const& resource = resources[i];
		writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.dstSet = m_handle;
		writeInfo.dstBinding = resource.bindingPoint;
		writeInfo.dstArrayElement = resource.arrayIndex;
		switch (resource.type)
		{
			case DescriptorType::UniformBuffer:
			{
				writeInfo.descriptorCount = resource.buffers.Size();
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				Vector<VkDescriptorBufferInfo>& buffers = bufferInfoList.emplace_back(resource.buffers.Size());
				for (uint32_t j = 0; j < resource.buffers.Size(); j++)
				{
					VkDescriptorBufferInfo& bufferInfo = buffers[j];
					BufferDescriptor const& buffer = resource.buffers[j];
					bufferInfo.buffer = buffer.buffer.GetHandle();
					bufferInfo.offset = buffer.offset;
					bufferInfo.range = buffer.size;
				}
				writeInfo.pBufferInfo = buffers.data();
				break;
			}
			case DescriptorType::CombinedImageSampler:
			{
				writeInfo.descriptorCount = resource.imageSamplers.Size();
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				Vector<VkDescriptorImageInfo>& images = imageInfoList.emplace_back(resource.imageSamplers.Size());
				for (uint32_t j = 0; j < resource.imageSamplers.Size(); j++)
				{
					VkDescriptorImageInfo& imageInfo = images[j];
					ImageSamplerDesciptor const& image = resource.imageSamplers[j];
					imageInfo.sampler = image.sampler.sampler.GetHandle();
					imageInfo.imageView = image.image.imageView.GetHandle();
					imageInfo.imageLayout = static_cast<VkImageLayout>(image.image.imageLayout);
				}
				writeInfo.pImageInfo = images.data();
				break;
			}
			case DescriptorType::SampledImage:
			{
				writeInfo.descriptorCount = resource.images.Size();
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				Vector<VkDescriptorImageInfo>& images = imageInfoList.emplace_back(resource.images.Size());
				for (uint32_t j = 0; j < resource.images.Size(); j++)
				{
					VkDescriptorImageInfo& imageInfo = images[j];
					ImageDescriptor const& image = resource.images[j];
					imageInfo.imageView = image.imageView.GetHandle();
					imageInfo.imageLayout = static_cast<VkImageLayout>(image.imageLayout);
				}
				writeInfo.pImageInfo = images.data();
				break;
			}
			case DescriptorType::Sampler:
			{
				writeInfo.descriptorCount = resource.samplers.Size();
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				Vector<VkDescriptorImageInfo>& images = imageInfoList.emplace_back(resource.samplers.Size());
				for (uint32_t j = 0; j < resource.samplers.Size(); j++)
				{
					VkDescriptorImageInfo& imageInfo = images[j];
					SamplerDescriptor const& sampler = resource.samplers[j];
					imageInfo.sampler = sampler.sampler.GetHandle();
				}
				writeInfo.pImageInfo = images.data();
				break;
			}
			default: Logger::Fatal("Invalid shader resource.");
		}
	}
	vkUpdateDescriptorSets(Context::GetDevice(), resources.Size(), descriptorInfo.data(), 0, nullptr);
}