#pragma once
#include "Core/Container/basic.h"
#include "Core/Memory/smart_ptr.h"
#include "Engine/Renderer/buffer.h"

namespace glex::render
{
	// Do not use this class anywhere else than FrameResource.
	class DynamicStagingBuffer : Uncopyable
	{
	private:
		struct BufferInfo
		{
			gl::Buffer buffer;
			gl::Memory memory;
			void* address;
			uint32_t filledSize;
		};

		Vector<BufferInfo> m_buffers;
		uint32_t m_bufferSize;
		uint32_t m_firstFree;

	public:
		DynamicStagingBuffer(uint32_t size);
		~DynamicStagingBuffer();
		DynamicStagingBuffer(DynamicStagingBuffer&& rhs) : m_buffers(std::move(rhs.m_buffers)), m_bufferSize(rhs.m_bufferSize), m_firstFree(rhs.m_firstFree) {}

		DynamicStagingBuffer& operator=(DynamicStagingBuffer&& rhs)
		{
			m_buffers.swap(rhs.m_buffers);
			m_bufferSize = rhs.m_bufferSize;
			m_firstFree = rhs.m_firstFree;
			return *this;
		}

		void Reset();
		bool UploadBuffer(WeakPtr<Buffer> dest, uint32_t offset, uint32_t size, void const* data);
	};
}