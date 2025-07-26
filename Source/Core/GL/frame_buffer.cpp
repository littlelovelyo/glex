#include "Core/GL/frame_buffer.h"
#include "Core/GL/context.h"

using namespace glex::gl;

bool FrameBuffer::Create(RenderPass renderPass, SequenceView<ImageView const> attachments, glm::uvec2 size)
{
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = renderPass.GetHandle();
	frameBufferInfo.attachmentCount = attachments.Size();
	frameBufferInfo.pAttachments = reinterpret_cast<VkImageView const*>(attachments.Data());
	frameBufferInfo.width = size.x;
	frameBufferInfo.height = size.y;
	frameBufferInfo.layers = 1;
	if (vkCreateFramebuffer(Context::GetDevice(), &frameBufferInfo, nullptr, &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void FrameBuffer::Destroy()
{
	vkDestroyFramebuffer(Context::GetDevice(), m_handle, nullptr);
}