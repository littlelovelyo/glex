/**
 * Vulkan descriptor manager.
 */
#pragma once

#include "Core/GL/descriptor.h"
#include <array>

namespace glex::render
{
	class DynamicDescriptorAllocator;
	class DescriptorAllocator : private Uncopyable
	{
	private:
		Vector<gl::DescriptorPool> m_freePools;
		Vector<gl::DescriptorPool> m_exaustedPools;
		DynamicDescriptorAllocator const& m_owner;

	public:
		DescriptorAllocator(DynamicDescriptorAllocator const& owner) : m_owner(owner) {}
		~DescriptorAllocator();
		DescriptorAllocator(DescriptorAllocator&& rhs) = default;
		DescriptorAllocator& operator=(DescriptorAllocator&& rhs) = default;
		gl::DescriptorSet AllocateDescriptorSet(gl::DescriptorSetLayout layout);
		void FreePools();
	};

	class DynamicDescriptorAllocator
	{
	private:
		uint32_t m_maxDescriptorSetCount;
		Vector<std::pair<gl::DescriptorType, uint32_t>> m_maxDescriptorCount;
		Vector<DescriptorAllocator> m_allocators;

	public:
		DynamicDescriptorAllocator(SequenceView<std::pair<gl::DescriptorType, uint32_t> const> maxCounts, uint32_t maxSets);
		uint32_t MaxDescriptorSetCount() const { return m_maxDescriptorSetCount; }
		auto const& MaxDescriptorCount() const { return m_maxDescriptorCount; }
		void Reset();
		gl::DescriptorSet AllocateDescriptorSet(gl::DescriptorSetLayout layout);
	};

	class StaticDescriptorAllocator : private Uncopyable
	{
	private:
		Vector<gl::DescriptorPool> m_freePools;
		Vector<gl::DescriptorPool> m_exaustedPools;
		uint32_t m_maxDescriptorSetCount;
		Vector<std::pair<gl::DescriptorType, uint32_t>> m_maxDescriptorCount;
		HashMap<VkDescriptorSet, gl::DescriptorPool> m_poolTable;

	public:
		StaticDescriptorAllocator(SequenceView<std::pair<gl::DescriptorType, uint32_t> const> maxCounts, uint32_t maxSets);
		~StaticDescriptorAllocator();
		gl::DescriptorSet AllocateDescriptorSet(gl::DescriptorSetLayout layout);
		void FreeDescriptorSet(gl::DescriptorSet set);
	};
}