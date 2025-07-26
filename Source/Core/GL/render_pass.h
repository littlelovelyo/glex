#pragma once
#include "Core/Container/sequence.h"
#include "Core/GL/image.h"
#include "Core/GL/enums.h"
#include <vulkan/vulkan.h>

namespace glex::gl
{
	struct AttachmentInfo
	{
		ImageFormat format = ImageFormat::RGBA;
		uint8_t msaaLevel = 0;
		AttachmentLoadOperation loadOp = AttachmentLoadOperation::Discard;
		AttachmentStoreOperation storeOp = AttachmentStoreOperation::Discard;
		AttachmentLoadOperation stencilLoadOp = AttachmentLoadOperation::Discard;
		AttachmentStoreOperation stencilStoreOp = AttachmentStoreOperation::Discard;
		ImageLayout beginLayout = ImageLayout::Undefined;
		ImageLayout endLayout = ImageLayout::Undefined;
	};

	struct SubpassInfo
	{
		SequenceView<std::pair<uint32_t, ImageLayout> const> inputAttachments;
		SequenceView<std::pair<uint32_t, ImageLayout> const> outputColorAttachments;
		std::pair<uint32_t, ImageLayout> outputDepthStencilAttachment = { UINT_MAX, ImageLayout::Undefined };
		SequenceView<uint32_t const> preserveAttachments;
	};

	struct DependencyInfo
	{
		uint32_t source = VK_SUBPASS_EXTERNAL;
		uint32_t dest = VK_SUBPASS_EXTERNAL;
		PipelineStage sourceStage = PipelineStage::None;
		PipelineStage destStage = PipelineStage::None;
		Access sourceAccess = Access::None;
		Access destAccess = Access::None;
	};

	class RenderPass
	{
	private:
		VkRenderPass m_handle;

	public:
		RenderPass() : m_handle(VK_NULL_HANDLE) {}
		RenderPass(VkRenderPass handle) : m_handle(handle) {}
		bool Create(SequenceView<AttachmentInfo const> attachments, SequenceView<SubpassInfo const> subpasses, SequenceView<DependencyInfo const> dependencies);
		void Destroy();
		VkRenderPass GetHandle() const { return m_handle; }
		bool operator==(RenderPass const& rhs) const { return m_handle == rhs.m_handle; };
	};
}