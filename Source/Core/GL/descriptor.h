#pragma once
#include "Core/GL/buffer.h"
#include "Core/GL/image.h"
#include "Core/Container/sequence.h"
#include <vulkan/vulkan.h>

namespace glex::gl
{
	struct BufferDescriptor
	{
		Buffer buffer;
		uint32_t offset;
		uint32_t size;
	};

	struct ImageDescriptor
	{
		ImageView imageView;
		ImageLayout imageLayout;
	};

	struct SamplerDescriptor
	{
		Sampler sampler;
	};

	struct ImageSamplerDesciptor
	{
		ImageDescriptor image;
		SamplerDescriptor sampler;
	};

	struct Descriptor
	{
		uint32_t bindingPoint = 0;
		uint32_t arrayIndex = 0;
		DescriptorType type = DescriptorType::UniformBuffer;
		union
		{
			SequenceView<BufferDescriptor const> buffers = {};
			SequenceView<ImageSamplerDesciptor const> imageSamplers;
			SequenceView<ImageDescriptor const> images;
			SequenceView<SamplerDescriptor const> samplers;
		};
		~Descriptor() {}
	};

	class DescriptorSet
	{
	private:
		VkDescriptorSet m_handle;

	public:
		DescriptorSet() : m_handle(VK_NULL_HANDLE) {}
		DescriptorSet(VkDescriptorSet handle) : m_handle(handle) {}
		VkDescriptorSet GetHandle() const { return m_handle; }
		bool operator==(DescriptorSet const& rhs) const = default;
		void BindDescriptors(SequenceView<Descriptor const> resources);
	};

	struct DescriptorBinding
	{
		uint32_t bindingPoint = 0;
		uint32_t arraySize = 1;
		DescriptorType type = DescriptorType::UniformBuffer;
		ShaderStage shaderStage = ShaderStage::AllGraphics;
	};

	class DescriptorSetLayout
	{
	private:
		VkDescriptorSetLayout m_handle;

	public:
		DescriptorSetLayout() : m_handle(VK_NULL_HANDLE) {}
		DescriptorSetLayout(VkDescriptorSetLayout handle) : m_handle(handle) {}
		bool Create(SequenceView<DescriptorBinding const> bindings);
		void Destroy();
		VkDescriptorSetLayout GetHandle() const { return m_handle; }
		bool operator==(DescriptorSetLayout const& rhs) const = default;
	};

	class DescriptorLayout
	{
	private:
		VkPipelineLayout m_handle;

	public:
		DescriptorLayout() : m_handle(VK_NULL_HANDLE) {}
		DescriptorLayout(VkPipelineLayout handle) : m_handle(handle) {}
		bool Create(SequenceView<DescriptorSetLayout const> descriptorSets, ShaderStage constantStages);
		void Destroy();
		VkPipelineLayout GetHandle() const { return m_handle; }
		bool operator==(DescriptorLayout const& rhs) const = default;
	};

	class DescriptorPool
	{
	private:
		VkDescriptorPool m_handle;

	public:
		DescriptorPool() : m_handle(VK_NULL_HANDLE) {};
		DescriptorPool(VkDescriptorPool handle) : m_handle(handle) {}
		bool Create(SequenceView<std::pair<DescriptorType, uint32_t> const> size, uint32_t maxNumSets, bool freeIndividual);
		void Destroy();
		VkDescriptorPool GetHandle() const { return m_handle; }
		bool operator==(DescriptorPool const& rhs) const = default;
		DescriptorSet AllocateDescriptorSet(DescriptorSetLayout layout);
		void FreeDescriptorSet(DescriptorSet descriptorSet);
		void Reset();
	};
}