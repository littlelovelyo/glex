#include "Core/GL/buffer.h"
#include "Core/GL/context.h"
#include <iostream>

using namespace glex::gl;

void* Memory::Map(uint32_t offset, uint32_t size)
{
	void* data;
	!vmaMapMemory(Context::GetAllocator(), m_handle, &data);
	return data;
}

void Memory::Unmap()
{
	vmaUnmapMemory(Context::GetAllocator(), m_handle);
}

void Memory::Flush(uint32_t offset, uint32_t size)
{
	vmaFlushAllocation(Context::GetAllocator(), m_handle, offset, size);
}

void Memory::Invalidate(uint32_t offset, uint32_t size)
{
	vmaInvalidateAllocation(Context::GetAllocator(), m_handle, offset, size);
}

Memory Buffer::Create(BufferUsage usage, uint32_t size, bool isStagingBuffer)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VulkanEnum::GetBufferUsage(usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	if (isStagingBuffer)
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	VmaAllocation allocation;
	if (vmaCreateBuffer(Context::GetAllocator(), &bufferInfo, &allocInfo, &m_handle, &allocation, nullptr) == VK_SUCCESS)
		return Memory(allocation);
	m_handle = VK_NULL_HANDLE;
	return Memory(VK_NULL_HANDLE);
}

void Buffer::Destroy(Memory memory)
{
	vmaDestroyBuffer(Context::GetAllocator(), m_handle, memory.GetHandle());
}