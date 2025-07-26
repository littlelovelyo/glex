#include "Engine/Renderer/staging_buffer.h"
#include "Engine/Renderer/renderer.h"
#include "Core/assert.h"

using namespace glex::render;

DynamicStagingBuffer::DynamicStagingBuffer(uint32_t size) : m_bufferSize(size)
{
	GLEX_DEBUG_ASSERT(Mem::IsAligned(size, sizeof(glm::mat4))) {};
	BufferInfo& buffer = m_buffers.emplace_back();
	// Cannot use wrapped Buffer here because it uses the deletion queue to destroy itself
	// while we get destroyed after deletion queue gets destroyed.
	buffer.memory = buffer.buffer.Create(gl::BufferUsage::TransferSource, size, true);
	if (buffer.memory.GetHandle() == VK_NULL_HANDLE)
		m_buffers.pop_back(); // Try again later.
	else
	{
		buffer.address = buffer.memory.Map(0, size);
		buffer.filledSize = 0;
	}
}

DynamicStagingBuffer::~DynamicStagingBuffer()
{
	for (BufferInfo& buffer : m_buffers)
		buffer.buffer.Destroy(buffer.memory);
}

void DynamicStagingBuffer::Reset()
{
	for (BufferInfo& buffer : m_buffers)
		buffer.filledSize = 0;
	m_firstFree = 0;
}

bool DynamicStagingBuffer::UploadBuffer(WeakPtr<Buffer> dest, uint32_t offset, uint32_t size, void const* data)
{
	GLEX_ASSERT_MSG(size < m_bufferSize, "Data is too large.") {}
	BufferInfo* availableBuffer;
	for (; m_firstFree < m_buffers.size(); m_firstFree++)
	{
		availableBuffer = &m_buffers[m_firstFree];
		if (m_bufferSize - availableBuffer->filledSize >= size)
			goto BUFFER_FOUND;
	}
	{
		availableBuffer = &m_buffers.emplace_back();
		availableBuffer->memory = availableBuffer->buffer.Create(gl::BufferUsage::TransferSource, m_bufferSize, true);
		if (availableBuffer->memory.GetHandle() == VK_NULL_HANDLE)
		{
			m_buffers.pop_back();
			Logger::Error("Cannot upload buffer. Shared VRAM ran out?");
			return false;
		}
		availableBuffer->address = availableBuffer->memory.Map(0, m_bufferSize);
		availableBuffer->filledSize = 0;
	}
BUFFER_FOUND:
	memcpy(Mem::Offset(availableBuffer->address, availableBuffer->filledSize), data, size);
	availableBuffer->memory.Flush(availableBuffer->filledSize, size);
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.CopyBuffer(availableBuffer->buffer, dest->GetBufferObject(), availableBuffer->filledSize, offset, size);
	availableBuffer->filledSize += size;
	return true;
}