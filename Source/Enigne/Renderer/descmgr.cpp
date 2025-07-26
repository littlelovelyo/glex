#include "Engine/Renderer/descmgr.h"
#include "Engine/Renderer/renderer.h"
#include "Core/assert.h"

using namespace glex;
using namespace glex::render;

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Single-frame allocator.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
gl::DescriptorSet DescriptorAllocator::AllocateDescriptorSet(gl::DescriptorSetLayout layout)
{
	while (!m_freePools.empty())
	{
		gl::DescriptorPool pool = m_freePools.back();
		gl::DescriptorSet descriptorSet = pool.AllocateDescriptorSet(layout);
		if (descriptorSet.GetHandle() != VK_NULL_HANDLE)
			return descriptorSet;
		m_freePools.pop_back();
		m_exaustedPools.push_back(pool);
	}
	gl::DescriptorPool pool;
	GLEX_ASSERT_MSG(pool.Create(m_owner.MaxDescriptorCount(), m_owner.MaxDescriptorSetCount(), false), "Descriptor pool exausted.") {}
	m_freePools.push_back(pool);
	gl::DescriptorSet descriptorSet = pool.AllocateDescriptorSet(layout);
	GLEX_ASSERT_MSG(descriptorSet.GetHandle() != VK_NULL_HANDLE, "Descriptor pool exausted.") {}
	return descriptorSet;
}

void DescriptorAllocator::FreePools()
{
	for (gl::DescriptorPool pool : m_freePools)
		pool.Reset();
	for (gl::DescriptorPool pool : m_exaustedPools)
	{
		pool.Reset();
		m_freePools.push_back(pool);
	}
	m_exaustedPools.clear();
}

DescriptorAllocator::~DescriptorAllocator()
{
	for (gl::DescriptorPool pool : m_freePools)
		Renderer::PendingDelete([=]() mutable { pool.Destroy(); });
	for (gl::DescriptorPool pool : m_exaustedPools)
		Renderer::PendingDelete([=]() mutable { pool.Destroy(); });
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Dynamic allocator.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
DynamicDescriptorAllocator::DynamicDescriptorAllocator(SequenceView<std::pair<gl::DescriptorType, uint32_t> const> maxCounts, uint32_t maxSets) :
	m_maxDescriptorSetCount(maxSets), m_maxDescriptorCount(maxCounts.begin(), maxCounts.end())
{
	uint32_t renderAheadCount = Renderer::GetRenderSettings().renderAheadCount;
	m_allocators.reserve(renderAheadCount);
	for (uint32_t i = 0; i < renderAheadCount; i++)
		m_allocators.emplace_back(*this);
}

void DynamicDescriptorAllocator::Reset()
{
	m_allocators[Renderer::CurrentFrame()].FreePools();
}

gl::DescriptorSet DynamicDescriptorAllocator::AllocateDescriptorSet(gl::DescriptorSetLayout layout)
{
	return m_allocators[Renderer::CurrentFrame()].AllocateDescriptorSet(layout);
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Static allocator.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
StaticDescriptorAllocator::StaticDescriptorAllocator(SequenceView<std::pair<gl::DescriptorType, uint32_t> const> maxCounts, uint32_t maxSets) :
	m_maxDescriptorSetCount(maxSets), m_maxDescriptorCount(maxCounts.begin(), maxCounts.end()) {}

StaticDescriptorAllocator::~StaticDescriptorAllocator()
{
	for (gl::DescriptorPool pool : m_freePools)
		Renderer::PendingDelete([=]() mutable { pool.Destroy(); });
	for (gl::DescriptorPool pool : m_exaustedPools)
		Renderer::PendingDelete([=]() mutable { pool.Destroy(); });
}

gl::DescriptorSet StaticDescriptorAllocator::AllocateDescriptorSet(gl::DescriptorSetLayout layout)
{
	while (!m_freePools.empty())
	{
		gl::DescriptorPool pool = m_freePools.back();
		gl::DescriptorSet descriptorSet = pool.AllocateDescriptorSet(layout);
		if (descriptorSet.GetHandle() != VK_NULL_HANDLE)
		{
			m_poolTable[descriptorSet.GetHandle()] = pool;
			return descriptorSet;
		}
		m_freePools.pop_back();
		m_exaustedPools.push_back(pool);
	}
	gl::DescriptorPool pool;
	GLEX_ASSERT_MSG(pool.Create(m_maxDescriptorCount, m_maxDescriptorSetCount, true), "Descriptor pool exausted.") {}
	m_freePools.push_back(pool);
	gl::DescriptorSet descriptorSet = pool.AllocateDescriptorSet(layout);
	GLEX_ASSERT_MSG(descriptorSet.GetHandle() != VK_NULL_HANDLE, "Descriptor pool exausted.") {}
	m_poolTable[descriptorSet.GetHandle()] = pool;
	return descriptorSet;
}

void StaticDescriptorAllocator::FreeDescriptorSet(gl::DescriptorSet set)
{
	gl::DescriptorPool pool = m_poolTable[set.GetHandle()];
	if (pool.GetHandle() == VK_NULL_HANDLE)
	{
		Logger::Error("Freed a descriptor set not allocated from this allocator.");
		m_poolTable.erase(set.GetHandle());
		return;
	}
	pool.FreeDescriptorSet(set);
	auto iter = eastl::find(m_exaustedPools.begin(), m_exaustedPools.end(), pool);
	if (iter != m_exaustedPools.end())
	{
		m_exaustedPools.erase_unsorted(iter);
		m_freePools.push_back(pool);
	}
	m_poolTable.erase(set.GetHandle());
}