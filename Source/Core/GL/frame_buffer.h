#pragma once
#include "Core/Container/sequence.h"
#include "Core/GL/render_pass.h"
#include "Core/GL/image.h"

namespace glex::gl
{
	class FrameBuffer
	{
	private:
		VkFramebuffer m_handle;

	public:
		FrameBuffer() : m_handle(VK_NULL_HANDLE) {}
		FrameBuffer(VkFramebuffer handle) : m_handle(handle) {}
		bool Create(RenderPass renderPass, SequenceView<ImageView const> attachments, glm::uvec2 size);
		void Destroy();
		VkFramebuffer GetHandle() const { return m_handle; }
	};
}