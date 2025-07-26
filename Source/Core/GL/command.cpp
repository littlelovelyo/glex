#include "Core/GL/command.h"
#include "Core/GL/context.h"

using namespace glex::gl;

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Command pool.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
bool CommandPool::Create(uint32_t queueFamily, bool transient)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (transient)
		poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = queueFamily;
	if (vkCreateCommandPool(Context::GetDevice(), &poolInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void CommandPool::Destroy()
{
	vkDestroyCommandPool(Context::GetDevice(), m_handle, Context::HostAllocator());
}

CommandBuffer CommandPool::AllocateCommandBuffer()
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = m_handle;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;
	CommandBuffer result;
	if (vkAllocateCommandBuffers(Context::GetDevice(), &info, reinterpret_cast<VkCommandBuffer*>(&result)) != VK_SUCCESS)
		result = VK_NULL_HANDLE;
	return result;
}

void CommandPool::FreeCommandBuffer(CommandBuffer commandBuffer)
{
	vkFreeCommandBuffers(Context::GetDevice(), m_handle, 1, reinterpret_cast<VkCommandBuffer*>(&commandBuffer));
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Command buffer.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
void CommandBuffer::Reset()
{
	!vkResetCommandBuffer(m_handle, 0);
}

void CommandBuffer::Begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	!vkBeginCommandBuffer(m_handle, &beginInfo);
}

void CommandBuffer::End()
{
	!vkEndCommandBuffer(m_handle);
}

void CommandBuffer::BeginRenderPass(RenderPass renderPass, FrameBuffer frameBuffer, glm::uvec2 size, SequenceView<ClearValue const> clearValues)
{
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass.GetHandle();
	renderPassInfo.framebuffer = frameBuffer.GetHandle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { size.x, size.y };
	renderPassInfo.clearValueCount = clearValues.Size();
	renderPassInfo.pClearValues = reinterpret_cast<VkClearValue const*>(clearValues.Data());
	vkCmdBeginRenderPass(m_handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::NextSubpass()
{
	vkCmdNextSubpass(m_handle, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(m_handle);
}

void CommandBuffer::BindPipelineState(PipelineState pipelineState)
{
	vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState.GetHandle());
}

void CommandBuffer::SetViewport(glm::vec4 const& border)
{
	VkViewport viewport = {};
	viewport.x = border.x;
	viewport.y = border.y;
	viewport.width = border.z;
	viewport.height = border.w;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_handle, 0, 1, &viewport);
}

void CommandBuffer::SetScissor(glm::uvec4 const& border)
{
	VkRect2D scissor = {};
	scissor.offset = { static_cast<int32_t>(border.x), static_cast<int32_t>(border.y) };
	scissor.extent = { border.z, border.w };
	vkCmdSetScissor(m_handle, 0, 1, &scissor);
}

void CommandBuffer::BindVertexBuffer(Buffer buffer, uint32_t offset)
{
	VkDeviceSize offset64 = offset;
	vkCmdBindVertexBuffers(m_handle, 0, 1, reinterpret_cast<VkBuffer*>(&buffer), &offset64);
}

void CommandBuffer::BindIndexBuffer(Buffer buffer, uint32_t offset)
{
	vkCmdBindIndexBuffer(m_handle, buffer.GetHandle(), offset, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindDescriptorSet(DescriptorLayout layout, uint32_t index, DescriptorSet descriptorSet)
{
	vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.GetHandle(), index, 1, reinterpret_cast<VkDescriptorSet*>(&descriptorSet), 0, nullptr);
}

void CommandBuffer::PushConstants(DescriptorLayout layout, ShaderStage stage, uint32_t offset, uint32_t size, void const* data)
{
	vkCmdPushConstants(m_handle, layout.GetHandle(), VulkanEnum::GetShaderStage(stage), offset, size, data);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset)
{
	vkCmdDraw(m_handle, vertexCount, instanceCount, vertexOffset, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t indexOffset)
{
	vkCmdDrawIndexed(m_handle, indexCount, instanceCount, indexOffset, vertexOffset, 0);
}

void CommandBuffer::ClearColorImage(Image image, ImageLayout layout, ClearValue clearColor)
{
	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	vkCmdClearColorImage(m_handle, image.GetHandle(), static_cast<VkImageLayout>(layout), reinterpret_cast<VkClearColorValue*>(&clearColor), 1, &range);
}

void CommandBuffer::BlitImage(Image source, Image dest, glm::uvec2 sourceSize, glm::uvec2 destSize, ImageFilter filter)
{
	VkImageBlit2 blitInfo = {};
	blitInfo.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	blitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitInfo.srcSubresource.mipLevel = 0;
	blitInfo.srcSubresource.baseArrayLayer = 0;
	blitInfo.srcSubresource.layerCount = 1;
	blitInfo.srcOffsets[0] = { 0, 0, 0 };
	blitInfo.srcOffsets[1] = { static_cast<int32_t>(sourceSize.x), static_cast<int32_t>(sourceSize.y), 1 };
	blitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitInfo.dstSubresource.mipLevel = 0;
	blitInfo.dstSubresource.baseArrayLayer = 0;
	blitInfo.dstSubresource.layerCount = 1;
	blitInfo.dstOffsets[0] = { 0, 0, 0 };
	blitInfo.dstOffsets[1] = { static_cast<int32_t>(destSize.x), static_cast<int32_t>(destSize.y), 1 };
	VkBlitImageInfo2 info = {};
	info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	info.srcImage = source.GetHandle();
	info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	info.dstImage = dest.GetHandle();
	info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	info.regionCount = 1;
	info.pRegions = &blitInfo;
	info.filter = static_cast<VkFilter>(filter);
	vkCmdBlitImage2(m_handle, &info);
}

void CommandBuffer::CopyBuffer(Buffer source, Buffer dest, uint32_t sourceOffset, uint32_t destOffset, uint32_t size)
{
	VkBufferCopy region = {};
	region.srcOffset = sourceOffset;
	region.dstOffset = destOffset;
	region.size = size;
	vkCmdCopyBuffer(m_handle, source.GetHandle(), dest.GetHandle(), 1, &region);
}

void CommandBuffer::CopyImage(Buffer source, uint32_t offset, Image dest, uint32_t layer, ImageAspect aspect, glm::uvec2 size)
{
	VkBufferImageCopy imageCopy = {};
	imageCopy.bufferOffset = offset;
	imageCopy.imageSubresource.aspectMask = VulkanEnum::GetImageAspect(aspect);
	imageCopy.imageSubresource.mipLevel = 0;
	imageCopy.imageSubresource.baseArrayLayer = layer;
	imageCopy.imageSubresource.layerCount = 1;
	imageCopy.imageOffset = { 0, 0, 0 };
	imageCopy.imageExtent = { size.x, size.y, 1 };
	vkCmdCopyBufferToImage(m_handle, source.GetHandle(), dest.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
}

void CommandBuffer::ExecutionBarrier(PipelineStage stageBefore, PipelineStage stageAfter)
{
	vkCmdPipelineBarrier(m_handle, static_cast<VkPipelineStageFlags2>(stageBefore), static_cast<VkPipelineStageFlags2>(stageAfter), 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

void CommandBuffer::MemoryBarrier(PipelineStage stageBefore, PipelineStage stageAfter, Access accessBefore, Access accessAfter)
{
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = static_cast<VkAccessFlags2>(accessBefore);
	memoryBarrier.dstAccessMask = static_cast<VkAccessFlags2>(accessAfter);
	vkCmdPipelineBarrier(m_handle, static_cast<VkPipelineStageFlags2>(stageBefore), static_cast<VkPipelineStageFlags2>(stageAfter), 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

void CommandBuffer::ImageMemoryBarrier(Image image, uint32_t layerIndex, uint32_t numLayers, ImageAspect aspect, PipelineStage stageBefore, Access accessBefore, ImageLayout oldLayout, PipelineStage stageAfter, Access accessAfter, ImageLayout newLayout)
{
	VkImageMemoryBarrier2 imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(stageBefore);
	imageBarrier.srcAccessMask = static_cast<VkAccessFlags2>(accessBefore);
	imageBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(stageAfter);
	imageBarrier.dstAccessMask = static_cast<VkAccessFlags2>(accessAfter);
	imageBarrier.oldLayout = VulkanEnum::GetImageLayout(oldLayout);
	imageBarrier.newLayout = VulkanEnum::GetImageLayout(newLayout);
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.image = image.GetHandle();
	imageBarrier.subresourceRange.aspectMask = VulkanEnum::GetImageAspect(aspect);
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	imageBarrier.subresourceRange.baseArrayLayer = layerIndex;
	imageBarrier.subresourceRange.layerCount = numLayers;
	VkDependencyInfo depInfo = {};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;
	vkCmdPipelineBarrier2(m_handle, &depInfo);
}

void CommandBuffer::BufferMemoryBarrier(Buffer buffer, uint32_t offset, uint32_t size, PipelineStage stageBefore, Access accessBefore, PipelineStage stageAfter, Access accessAfter)
{
	VkBufferMemoryBarrier2 bufferBarrier = {};
	bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	bufferBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(stageBefore);
	bufferBarrier.srcAccessMask = static_cast<VkAccessFlags2>(accessBefore);
	bufferBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(stageAfter);
	bufferBarrier.dstAccessMask = static_cast<VkAccessFlags2>(accessAfter);
	bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.buffer = buffer.GetHandle();
	bufferBarrier.offset = offset;
	bufferBarrier.size = size;
	VkDependencyInfo depInfo = {};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.bufferMemoryBarrierCount = 1;
	depInfo.pBufferMemoryBarriers = &bufferBarrier;
	vkCmdPipelineBarrier2(m_handle, &depInfo);
}