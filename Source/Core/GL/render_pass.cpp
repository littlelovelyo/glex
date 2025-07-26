#include "Core/GL/render_pass.h"
#include "Core/GL/context.h"
#include "Core/Container/basic.h"

using namespace glex::gl;

bool RenderPass::Create(SequenceView<AttachmentInfo const> attachments, SequenceView<SubpassInfo const> subpasses, SequenceView<DependencyInfo const> dependencies)
{
	// Attachment descriptions.
	Vector<VkAttachmentDescription> attachmentDescriptions(attachments.Size());
	for (uint32_t i = 0; i < attachments.Size(); i++)
	{
		AttachmentInfo const& desc = attachments[i];
		VkAttachmentDescription& attachment = attachmentDescriptions[i];
		attachment.format = VulkanEnum::GetImageFormat(desc.format);
		attachment.samples = static_cast<VkSampleCountFlagBits>(desc.msaaLevel);
		/*constexpr auto getOp = [](AttachmentUsage usage) -> std::pair<VkAttachmentLoadOp, VkAttachmentStoreOp>
		{
			switch (usage)
			{
				case AttachmentUsage::Discard: return { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE };
				case AttachmentUsage::DiscardAndWrite: return { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE };
				case AttachmentUsage::ClearAndWrite: return { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE };
				case AttachmentUsage::ReadAndWrite: case AttachmentUsage::ReadAndKeep: return { VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE };
				case AttachmentUsage::ReadAndDiscard: return { VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE };
				default: Logger::Fatal("Invalid usage!"); std::unreachable();
			}
		};*/
		/*std::tie(attachment.loadOp, attachment.storeOp) = getOp(desc.usage);
		std::tie(attachment.stencilLoadOp, attachment.stencilStoreOp) = getOp(desc.usage);*/
		attachment.loadOp = VulkanEnum::GetAttachmentLoadOperation(desc.loadOp);
		attachment.storeOp = VulkanEnum::GetAttachmentStoreOperation(desc.storeOp);
		attachment.stencilLoadOp = VulkanEnum::GetAttachmentLoadOperation(desc.stencilLoadOp);
		attachment.stencilStoreOp = VulkanEnum::GetAttachmentStoreOperation(desc.stencilStoreOp);
		attachment.initialLayout = static_cast<VkImageLayout>(desc.beginLayout);
		attachment.finalLayout = static_cast<VkImageLayout>(desc.endLayout);
	}

	// Subpass descriptions.
	Vector<VkSubpassDescription> subpassDescriptions(subpasses.Size());
	Vector<Vector<VkAttachmentReference>> inputRefs;
	Vector<Vector<VkAttachmentReference>> colorRefs;
	Vector<VkAttachmentReference> depthRefs;
	constexpr auto emplaceRefs = [](Vector<VkAttachmentReference>& outVector, SequenceView<AttachmentInfo const> attachments, SequenceView<std::pair<uint32_t, ImageLayout> const> refs)
	{
		outVector.resize(refs.Size());
		for (uint32_t i = 0; i < refs.Size(); i++)
		{
			outVector[i].attachment = refs[i].first;
			outVector[i].layout = static_cast<VkImageLayout>(refs[i].second);
		}
	};
	for (uint32_t i = 0; i < subpasses.Size(); i++)
	{
		VkSubpassDescription& subpass = subpassDescriptions[i];
		SubpassInfo const& desc = subpasses[i];
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		if (desc.inputAttachments.Size() != 0)
		{
			auto& list = inputRefs.emplace_back();
			emplaceRefs(list, attachments, desc.inputAttachments);
			subpass.inputAttachmentCount = desc.inputAttachments.Size();
			subpass.pInputAttachments = list.data();
		}
		if (desc.outputColorAttachments.Size() != 0)
		{
			auto& list = colorRefs.emplace_back();
			emplaceRefs(list, attachments, desc.outputColorAttachments);
			subpass.colorAttachmentCount = desc.outputColorAttachments.Size();
			subpass.pColorAttachments = list.data();
		}
		if (desc.outputDepthStencilAttachment.first != UINT_MAX)
		{
			VkAttachmentReference& ref = depthRefs.emplace_back();
			ref.attachment = desc.outputDepthStencilAttachment.first;
			ref.layout = static_cast<VkImageLayout>(desc.outputDepthStencilAttachment.second);
			subpass.pDepthStencilAttachment = &ref;
		}
		subpass.preserveAttachmentCount = desc.preserveAttachments.Size();
		subpass.pPreserveAttachments = desc.preserveAttachments.Data();
	}

	// Dependencies.
	Vector<VkSubpassDependency> dependencyDescriptions(dependencies.Size());
	for (uint32_t i = 0; i < dependencies.Size(); i++)
	{
		VkSubpassDependency& dependency = dependencyDescriptions[i];
		DependencyInfo const& desc = dependencies[i];
		dependency.srcSubpass = desc.source;
		dependency.dstSubpass = desc.dest;
		dependency.srcStageMask = static_cast<VkPipelineStageFlags>(desc.sourceStage);
		dependency.dstStageMask = static_cast<VkPipelineStageFlags>(desc.destStage);
		dependency.srcAccessMask = static_cast<VkAccessFlagBits>(desc.sourceAccess);
		dependency.dstAccessMask = static_cast<VkAccessFlagBits>(desc.destAccess);
	}

	// Create render pass.
	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = attachments.Size();
	info.pAttachments = attachmentDescriptions.data();
	info.subpassCount = subpasses.Size();
	info.pSubpasses = subpassDescriptions.data();
	info.dependencyCount = dependencies.Size();
	info.pDependencies = dependencyDescriptions.data();
	if (vkCreateRenderPass(Context::GetDevice(), &info, nullptr, &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void RenderPass::Destroy()
{
	vkDestroyRenderPass(Context::GetDevice(), m_handle, nullptr);
}