#pragma once
#include "Core/commdefs.h"
#include "Core/GL/enums.h"
#include <vma/vk_mem_alloc.h>

namespace glex::gl
{
	class Memory
	{
	private:
		VmaAllocation m_handle;

	public:
		Memory() : m_handle(VK_NULL_HANDLE) {};
		Memory(VmaAllocation handle) : m_handle(handle) {}
		VmaAllocation GetHandle() const { return m_handle; }
		void* Map(uint32_t offset, uint32_t size);
		void Unmap();
		void Flush(uint32_t offset, uint32_t size);
		void Invalidate(uint32_t offset, uint32_t size);
	};

	class Buffer
	{
	private:
		VkBuffer m_handle;

	public:
		Buffer() : m_handle(VK_NULL_HANDLE) {}
		Buffer(VkBuffer handle) : m_handle(handle) {}
		Memory Create(BufferUsage usage, uint32_t size, bool isStagingBuffer);
		void Destroy(Memory memory);
		VkBuffer GetHandle() const { return m_handle; }
	};
}