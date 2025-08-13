#include "Engine/Renderer/render_pass.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/GUI/batch.h"
#include "Core/GL/enums.h"
#include "Core/log.h"
#include "game.h"

using namespace glex;

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Builder.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
uint32_t RenderPass::Builder::GetOrAddAttachment(WeakPtr<ImageView> attachment)
{
	auto [iter, inserted] = m_attachmentIndices.insert({ attachment, m_attachments.size() });
	if (inserted)
	{
		AttachmentInternal& attach = m_attachments.emplace_back();
		attach.attachment = attachment;
		attach.format = attachment->GetImage()->Format();
		attach.samples = attachment->GetImage()->SampleCount();
		if (m_renderAera == glm::uvec2(0.0f, 0.0f))
			m_renderAera = attachment->GetImage()->Size();
		else if (m_renderAera != glm::uvec2(attachment->GetImage()->Size()))
			Logger::Error("Render pass must contain attachments of the same size.");
	}
	return iter->second;
}

void RenderPass::Builder::PushSubpass()
{
	m_subpasses.emplace_back();
}

void RenderPass::Builder::Read(WeakPtr<ImageView> attachment)
{
	uint32_t currentSubpass = m_attachments.size() - 1;
	uint32_t idxAttach = GetOrAddAttachment(attachment);
	AttachmentInternal& attach = m_attachments[idxAttach];

	// Read after write is not OK.
	GLEX_DEBUG_ASSERT(attach.lastWriteSubpass != currentSubpass) {}

	int32_t lastAccessedSubpass = glm::max(static_cast<int32_t>(attach.lastReadSubpass), static_cast<int32_t>(attach.lastWriteSubpass));
	for (int32_t i = currentSubpass - 1; i > lastAccessedSubpass; i--)
	{
		SubpassInternal& subpass = m_subpasses[i];
		subpass.passThroughs.push_back(idxAttach);
	}

	// Read-after-read is OK. Read-after-write requries a dependency.
	// If this attachment is written by some subpass...
	// We can only use input attachment in fragment shader. We should almost never read that in other shaders anyway.
	if (attach.lastWriteSubpass != UINT_MAX)
	{
		gl::DependencyInfo dep;
		dep.source = attach.lastWriteSubpass;
		dep.dest = currentSubpass;
		if (gl::VulkanEnum::IsColorFormat(attach.format))
		{
			dep.sourceStage = gl::PipelineStage::ColorOutput;
			dep.sourceAccess = gl::Access::ColorWrite;
			dep.destAccess = gl::Access::ColorRead;
		}
		else
		{
			dep.sourceStage = gl::PipelineStage::DepthStencilOutput;
			dep.sourceAccess = gl::Access::DepthStencilWrite;
			dep.destAccess = gl::Access::DepthStencilRead;
		}
		dep.destStage = gl::PipelineStage::FragmentShader;
		m_dependencies.push_back(dep);
	}

	SubpassInternal& subpass = m_subpasses.back();
	subpass.inputs.emplace_back(idxAttach, gl::ImageLayout::ShaderRead);
	if (attach.firstLoadOp == LOAD_OPERATION_UNSET)
		attach.firstLoadOp = gl::AttachmentLoadOperation::Load;
	attach.lastReadSubpass = currentSubpass;
}

void RenderPass::Builder::Write(WeakPtr<ImageView> attachment)
{
	uint32_t currentSubpass = m_attachments.size() - 1;
	uint32_t idxAttach = GetOrAddAttachment(attachment);
	AttachmentInternal& attach = m_attachments[idxAttach];

	if (attach.lastWriteSubpass != currentSubpass)
	{
		SubpassInternal& subpass = m_subpasses.back();
		if (gl::VulkanEnum::IsColorFormat(attach.format))
			subpass.colorOutputs.emplace_back(idxAttach, gl::ImageLayout::ColorAttachment);
		else
		{
			GLEX_DEBUG_ASSERT(subpass.depthStencilOutputs.first == UINT_MAX) {}
			subpass.depthStencilOutputs = { idxAttach, gl::ImageLayout::DepthStencilAttachment };
		}

		if (attach.firstLoadOp == LOAD_OPERATION_UNSET)
			attach.firstLoadOp = gl::AttachmentLoadOperation::Discard;

		// Write after write is not OK.
		GLEX_DEBUG_ASSERT(static_cast<int32_t>(attach.lastWriteSubpass) < static_cast<int32_t>(attach.lastReadSubpass) || attach.lastWriteSubpass == UINT_MAX) {}

		if (attach.lastReadSubpass != UINT_MAX)
		{
			// Write immediately after read is probably not OK.
			GLEX_DEBUG_ASSERT(attach.lastReadSubpass != currentSubpass) {}
			gl::DependencyInfo dep;
			dep.source = attach.lastReadSubpass;
			dep.dest = currentSubpass;
			dep.sourceStage = gl::PipelineStage::FragmentShader;
			if (gl::VulkanEnum::IsColorFormat(attach.format))
			{
				dep.sourceAccess = gl::Access::ColorRead;
				dep.destStage = gl::PipelineStage::ColorOutput;
				dep.destAccess = gl::Access::ColorWrite;
			}
			else
			{
				dep.sourceAccess = gl::Access::DepthStencilRead;
				dep.destStage = gl::PipelineStage::DepthStencilOutput;
				dep.destAccess = gl::Access::DepthStencilWrite;
			}
			m_dependencies.push_back(dep);
		}
		attach.lastWriteSubpass = currentSubpass;
	}
}

void RenderPass::Builder::Clear(WeakPtr<ImageView> attachment)
{
	AttachmentInternal& attach = m_attachments[GetOrAddAttachment(attachment)];

	// Clear after read/write is not OK.
	GLEX_DEBUG_ASSERT(attach.firstLoadOp == LOAD_OPERATION_UNSET) {}

	// Clear implicitly means write.
	attach.firstLoadOp = gl::AttachmentLoadOperation::Clear;
	Write(attachment);
}

void RenderPass::Builder::Output(WeakPtr<ImageView> attachment)
{
	uint32_t idxAttach = GetOrAddAttachment(attachment);
	AttachmentInternal& attach = m_attachments[idxAttach];

	int32_t lastAccessedSubpass = glm::max(static_cast<int32_t>(attach.lastReadSubpass), static_cast<int32_t>(attach.lastWriteSubpass));
	for (int32_t i = m_subpasses.size() - 1; i > lastAccessedSubpass; i--)
	{
		SubpassInternal& subpass = m_subpasses[i];
		subpass.passThroughs.push_back(idxAttach);
	}

	// Why can an attachment not be used?
	// If it is written or read by *someone*, set the store operation.
	// The sequence will be write — output or read — output.
	GLEX_DEBUG_ASSERT(attach.firstLoadOp != LOAD_OPERATION_UNSET) {}
	attach.lastStoreOp = gl::AttachmentStoreOperation::Store;
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Render pass cache.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
gl::RenderPass RenderPass::GetCachedOrCreateNewRenderPass(Builder const& builder)
{
	alignas(4) uint8_t description[Limits::RENDER_PASS_SEQUENCE_LENGTH];
	uint32_t pointer = 0;

	// Size check.
	uint32_t numAttachments = builder.GetAttachments().size();
	uint32_t requiredSize = numAttachments * 2 + 4;
	if (requiredSize > Limits::RENDER_PASS_SEQUENCE_LENGTH)
	{
		Logger::Error("Render pass is too large.");
		return gl::RenderPass();
	}

	// This won't be a human-readable string anymore.
	// Mimic the building process again.
	for (AttachmentInternal const& attach : builder.GetAttachments())
	{
		description[pointer++] = *attach.format;
		description[pointer++] = attach.samples;
	}
	// Make them 4-byte aligned.
	description[pointer++] = 0xff;
	description[pointer++] = 0xff;
	if (!(numAttachments & 1))
	{
		description[pointer++] = 0xff;
		description[pointer++] = 0xff;
	}

	for (SubpassInternal const& subpass : builder.GetSubpasses())
	{
		requiredSize = (subpass.inputs.size() + 1 + subpass.colorOutputs.size() + 2 + subpass.passThroughs.size() + 1) * 4;
		if (pointer + requiredSize > Limits::RENDER_PASS_SEQUENCE_LENGTH)
		{
			Logger::Error("Render pass is too large.");
			return gl::RenderPass();
		}

		for (auto [i, layout] : subpass.inputs)
		{
			*reinterpret_cast<uint32_t*>(description + pointer) = i;
			pointer += 4;
		}
		*reinterpret_cast<uint32_t*>(description + pointer) = UINT_MAX;
		pointer += 4;

		for (auto [i, layout] : subpass.colorOutputs)
		{
			*reinterpret_cast<uint32_t*>(description + pointer) = i;
			pointer += 4;
		}
		*reinterpret_cast<uint32_t*>(description + pointer) = UINT_MAX;
		pointer += 4;
		*reinterpret_cast<uint32_t*>(description + pointer) = subpass.depthStencilOutputs.first;
		pointer += 4;

		for (uint32_t i : subpass.passThroughs)
		{
			*reinterpret_cast<uint32_t*>(description + pointer) = i;
			pointer += 4;
		}
		*reinterpret_cast<uint32_t*>(description + pointer) = UINT_MAX;
		pointer += 4;
	}

	auto [iter, inserted] = s_renderPassCache.insert(SequenceView<uint32_t>(reinterpret_cast<uint32_t*>(description), pointer / 4));
	if (!inserted)
	{
		gl::RenderPass result = iter->second;
		s_refCounts[result.GetHandle()].second++;
		return result;
	}

	Vector<gl::AttachmentInfo> attachments(builder.GetAttachments().size());
	for (uint32_t i = 0; i < builder.GetAttachments().size(); i++)
	{
		gl::AttachmentInfo& attachmentInfo = attachments[i];
		AttachmentInternal const& attach = builder.GetAttachments()[i];
		attachmentInfo.format = attach.format;
		attachmentInfo.msaaLevel = attach.samples;
		attachmentInfo.loadOp = attach.firstLoadOp;
		attachmentInfo.storeOp = attach.lastStoreOp == STORE_OPERATION_UNSET ? gl::AttachmentStoreOperation::Discard : attach.lastStoreOp;
		// Stencil is skipped.
		// Layout can be deduced from read/write order.
		attachmentInfo.beginLayout = attach.initialLayout;
		attachmentInfo.endLayout = attach.finalLayout;
	}
	Vector<gl::SubpassInfo> subpasses(builder.GetSubpasses().size());
	for (uint32_t i = 0; i < builder.GetSubpasses().size(); i++)
	{
		gl::SubpassInfo& subpassInfo = subpasses[i];
		SubpassInternal const& subpass = builder.GetSubpasses()[i];
		subpassInfo.inputAttachments = subpass.inputs;
		subpassInfo.outputColorAttachments = subpass.colorOutputs;
		subpassInfo.outputDepthStencilAttachment = subpass.depthStencilOutputs;
		subpassInfo.preserveAttachments = subpass.passThroughs;
	}
	gl::RenderPass renderPass;
	if (!renderPass.Create(attachments, subpasses, builder.GetDependencies()))
	{
		Logger::Error("Cannot create render pass.");
		return gl::RenderPass();
	}

	uint32_t* buffer = Mem::Alloc<uint32_t>(pointer / 4);
	memcpy(buffer, description, pointer);
	SequenceView<uint32_t> key(buffer, pointer / 4);
	s_refCounts[renderPass.GetHandle()] = { key, 1 };
	iter->second = renderPass;
	return renderPass;
}

void RenderPass::FreeRenderPass(gl::RenderPass renderPass)
{
	auto iter = s_refCounts.find(renderPass.GetHandle());
	GLEX_DEBUG_ASSERT(iter != s_refCounts.end()) {}
	if (--iter->second.second == 0)
	{
		SequenceView<uint32_t> key = iter->second.first;
		s_refCounts.erase(iter);
		Renderer::PendingDelete([=]() mutable
		{
			renderPass.Destroy();
		});
		s_renderPassCache.erase(key);
		Mem::Free(key.Data());
	}
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Instance members.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
bool RenderPass::EndRenderPassDefinition(Builder& builder)
{
	GLEX_DEBUG_ASSERT(m_renderPassObject.GetHandle() == VK_NULL_HANDLE) {}
	for (uint32_t i = 0; i < builder.GetAttachments().size(); i++)
	{
		AttachmentInternal& attach = builder.GetAttachments()[i];
		gl::ImageLayout readOptimalLayout = gl::VulkanEnum::IsColorFormat(attach.format) ? gl::ImageLayout::ShaderRead : gl::ImageLayout::DepthStencilRead;
		gl::ImageLayout writeOptimalLayout = gl::VulkanEnum::IsColorFormat(attach.format) ? gl::ImageLayout::ColorAttachment : gl::ImageLayout::DepthStencilAttachment;
		if (attach.firstLoadOp == gl::AttachmentLoadOperation::Load)
			attach.initialLayout = readOptimalLayout;
		else
			attach.initialLayout = writeOptimalLayout;
		attach.finalLayout = static_cast<int32_t>(attach.lastReadSubpass) <= static_cast<int32_t>(attach.lastWriteSubpass) ? writeOptimalLayout : readOptimalLayout;
	}

	m_renderPassObject = GetCachedOrCreateNewRenderPass(builder);
	if (m_renderPassObject.GetHandle() != VK_NULL_HANDLE)
	{
		m_renderAera = builder.GetRenderAera();
		Vector<gl::ImageView> attachments(builder.GetAttachments().size());
		for (uint32_t i = 0; i < builder.GetAttachments().size(); i++)
			attachments[i] = builder.GetAttachments()[i].attachment->GetImageViewObject();
		m_attachments.resize(builder.GetAttachments().size());
		for (uint32_t i = 0; i < builder.GetAttachments().size(); i++)
		{
			AttachmentInternal const& attach = builder.GetAttachments()[i];
			m_attachments[i] = { attach.attachment, attach.initialLayout, attach.finalLayout };
		}
		return m_frameBuffer.Create(m_renderPassObject, attachments, m_renderAera);
		// Keep render pass reference because we can recreate frame buffer later.
	}
	return false;
}

RenderPass::~RenderPass()
{
	if (m_renderPassObject.GetHandle() != VK_NULL_HANDLE)
	{
		FreeRenderPass(m_renderPassObject);
		if (m_frameBuffer.GetHandle() != VK_NULL_HANDLE)
			Renderer::PendingDelete([fb = m_frameBuffer]() mutable { fb.Destroy(); });
	}
}

void RenderPass::Invalidate()
{
	if (IsValid())
	{
		Renderer::PendingDelete([fb = m_frameBuffer]() mutable { fb.Destroy(); });
		m_frameBuffer = VK_NULL_HANDLE;
	}
}

bool RenderPass::Recreate()
{
	GLEX_ASSERT(IsValid()) {}
	Renderer::PendingDelete([fb = m_frameBuffer]() mutable { fb.Destroy(); });
	Vector<gl::ImageView> imageViews(m_attachments.size());
	for (uint32_t i = 0; i < m_attachments.size(); i++)
		imageViews[i] = m_attachments[i].imageView->GetImageViewObject();
	m_renderAera = m_attachments[0].imageView->GetImage()->Size();
	if (m_frameBuffer.Create(m_renderPassObject, imageViews, m_renderAera))
		return true;
	Logger::Error("Frame buffer recreation failed. Cannot continue.");
	return false;
}

void RenderPass::BeginRenderPass(SequenceView<gl::ClearValue const> clearValues)
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();

	// Automatic layout transition.
	for (AttachmentInformation const& attach : m_attachments)
	{
		uint32_t layer = attach.imageView->LayerIndex();
		uint32_t numLayers = attach.imageView->LayerCount();
		WeakPtr<Image> image = attach.imageView->GetImage();
		for (uint32_t i = 0; i < numLayers; i++)
		{
			gl::ImageLayout currentLayout = image->GetImageLayout(layer + i);
			if (currentLayout != attach.initialLayout)
			{
				Renderer::AutomaticLayoutTransition(commandBuffer, image, attach.imageView->Aspect(), layer, numLayers, currentLayout, attach.initialLayout);
				break;
			}
		}
	}

	commandBuffer.BeginRenderPass(m_renderPassObject, m_frameBuffer, m_renderAera, clearValues);
	commandBuffer.SetViewport(glm::vec4(0.0f, 0.0f, m_renderAera.x, m_renderAera.y));
	commandBuffer.SetScissor(glm::uvec4(0, 0, m_renderAera.x, m_renderAera.y));
}

void RenderPass::NextSubpass()
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.NextSubpass();
}

void RenderPass::EndRenderPass()
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.EndRenderPass();

	for (AttachmentInformation const& attach : m_attachments)
	{
		uint32_t layer = attach.imageView->LayerIndex();
		uint32_t numLayers = attach.imageView->LayerCount();
		WeakPtr<Image> image = attach.imageView->GetImage();
		image->SetImageLayout(layer, numLayers, attach.finalLayout);
	}
}

void RenderPass::BindObjectData(void const* data, uint32_t size)
{
	// Even though we got 256, we don't have a custom shader compiler so it's hard to use more.
	uint32_t pushConstantsSize = glm::min<uint32_t>(size, 128);
	uint32_t exceedSize = size - pushConstantsSize;
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	WeakPtr<Shader> shader = Renderer::GetCurrentMaterialInstance()->GetShader();
	commandBuffer.PushConstants(shader->GetDescriptorLayout(), gl::ShaderStage::AllGraphics, 0, size, data);
	if (exceedSize != 0)
		Logger::Error("Object data more than 128 bytes not supported yet!");
}

void RenderPass::DrawAllControls()
{
	ui::BatchRenderer::BeginUIPass();
	auto& controlStack = GameInstance::GetControlStack();
	for (UniquePtr<ui::Control> const& control : controlStack)
		ui::BatchRenderer::PaintControl(control);
	auto& popupStack = GameInstance::GetPopupStack();
	for (auto const& [popup, pos] : popupStack)
		ui::BatchRenderer::PaintPopup(popup, pos);
	ui::BatchRenderer::EndUIPass();
}