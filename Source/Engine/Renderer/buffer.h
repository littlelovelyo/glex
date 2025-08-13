#pragma once
#include "Core/GL/buffer.h"

namespace glex
{
	class Buffer : private Unmoveable
	{
	private:
		gl::Buffer m_bufferObject;
		gl::Memory m_bufferMemory;
		uint32_t m_size;
		gl::BufferUsage m_usages;
		bool m_hostVisible;

	public:
		Buffer(gl::BufferUsage usages, uint32_t size, bool hostVisible);
		~Buffer();
		bool IsValid() const { return m_bufferMemory.GetHandle() != VK_NULL_HANDLE; }
		gl::Buffer GetBufferObject() const { return m_bufferObject; }
		gl::Memory GetMemoryObject() const { return m_bufferMemory; }
		uint32_t Size() const { return m_size; }
		void MemoryBarrier(gl::PipelineStage stageBefore, gl::Access accessBefore, gl::PipelineStage stageAfter, gl::Access accessAfter);
		bool Resize(uint32_t size);
		void* Map() { return m_bufferMemory.Map(0, m_size); }
		void Unmap() { m_bufferMemory.Unmap(); }
	};
}