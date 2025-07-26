#pragma once
#include "Core/commdefs.h"
#include "Core/GL/frame_buffer.h"
#include "Core/GL/descriptor.h"
#include "Core/GL/pipeline_state.h"
#include <vulkan/vulkan.h>

namespace glex::gl
{
	union ClearValue
	{
		union
		{
			glm::vec4 fColor;
			glm::uvec4 uColor;
		};
		struct
		{
			float depth;
			uint32_t stencil;
		};
	};

	class CommandBuffer
	{
	private:
		VkCommandBuffer m_handle;

	public:
		CommandBuffer() : m_handle(VK_NULL_HANDLE) {}
		CommandBuffer(VkCommandBuffer handle) : m_handle(handle) {}
		VkCommandBuffer GetHandle() const { return m_handle; }
		void Reset();
		void Begin();
		void End();
		void BeginRenderPass(RenderPass renderPass, FrameBuffer frameBuffer, glm::uvec2 size, SequenceView<ClearValue const> clearValues);
		void NextSubpass();
		void EndRenderPass();
		void BindPipelineState(PipelineState pipelineState);
		void SetViewport(glm::vec4 const& border);
		void SetScissor(glm::uvec4 const& border);
		void BindVertexBuffer(Buffer buffer, uint32_t offset);
		void BindIndexBuffer(Buffer buffer, uint32_t offset);
		void BindDescriptorSet(DescriptorLayout layout, uint32_t index, DescriptorSet descriptorSet);
		void PushConstants(DescriptorLayout layout, ShaderStage stage, uint32_t offset, uint32_t size, void const* data);
		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t indexOffset);
		void ClearColorImage(Image image, ImageLayout layout, ClearValue clearColor);
		void BlitImage(Image source, Image dest, glm::uvec2 sourceSize, glm::uvec2 destSize, ImageFilter filter);
		void CopyBuffer(Buffer source, Buffer dest, uint32_t sourceOffset, uint32_t destOffset, uint32_t size);
		void CopyImage(Buffer source, uint32_t offset, Image dest, uint32_t layer, ImageAspect aspect, glm::uvec2 size);
		void ExecutionBarrier(PipelineStage stageBefore, PipelineStage stageAfter);
		void MemoryBarrier(PipelineStage stageBefore, PipelineStage stageAfter, Access accessBefore, Access accessAfter);
		void ImageMemoryBarrier(Image image, uint32_t layerIndex, uint32_t numLayers, ImageAspect aspect, PipelineStage stageBefore, Access accessBefore, ImageLayout oldLayout, PipelineStage stageAfter, Access accessAfter, ImageLayout newLayout);
		void BufferMemoryBarrier(Buffer buffer, uint32_t offset, uint32_t size, PipelineStage stageBefore, Access accessBefore, PipelineStage stageAfter, Access accessAfter);
	};

	class CommandPool
	{
	private:
		VkCommandPool m_handle;

	public:
		CommandPool() : m_handle(VK_NULL_HANDLE) {}
		CommandPool(VkCommandPool handle) : m_handle(handle) {}
		bool Create(uint32_t queueFamily, bool transient);
		void Destroy();
		VkCommandPool GetHandle() const { return m_handle; }
		CommandBuffer AllocateCommandBuffer();
		void FreeCommandBuffer(CommandBuffer commandBuffer);
	};
}