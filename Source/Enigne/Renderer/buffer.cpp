#include "Engine/Renderer/buffer.h"
#include "Engine/Renderer/renderer.h"
#include "Core/GL/context.h"

using namespace glex;

Buffer::Buffer(gl::BufferUsage usages, uint32_t size, bool hostVisible) : m_size(size), m_usages(usages), m_hostVisible(hostVisible)
{
	m_bufferMemory = m_bufferObject.Create(usages, size, hostVisible);
}

Buffer::~Buffer()
{
	if (m_bufferMemory.GetHandle() != VK_NULL_HANDLE)
	{
		Renderer::PendingDelete([buf = m_bufferObject, mem = m_bufferMemory]() mutable
		{
			buf.Destroy(mem);
		});
	}
}

void Buffer::MemoryBarrier(gl::PipelineStage stageBefore, gl::Access accessBefore, gl::PipelineStage stageAfter, gl::Access accessAfter)
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.BufferMemoryBarrier(m_bufferObject, 0, m_size, stageBefore, accessBefore, stageAfter, accessAfter);
}

bool Buffer::Resize(uint32_t size)
{
	GLEX_DEBUG_ASSERT(IsValid()) {}
	gl::Buffer newBuffer;
	gl::Memory newMemory = newBuffer.Create(m_usages, size, m_hostVisible);
	if (newMemory.GetHandle() == VK_NULL_HANDLE)
		return false;
	Renderer::PendingDelete([buf = m_bufferObject, mem = m_bufferMemory]() mutable { buf.Destroy(mem); });
	m_bufferObject = newBuffer;
	m_bufferMemory = newMemory;
	return true;
}