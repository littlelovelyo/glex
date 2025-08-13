#pragma once
#include "Core/commdefs.h"
#include "Core/GL/context.h"
#include "Core/Container/function.h"
#include "Core/Memory/smart_ptr.h"
#include "Core/GL/pipeline_state.h"
#include "Engine/Scripting/lib.h"
#include "Engine/Renderer/buffer.h"
#include "Engine/Renderer/image.h"
#include "Engine/Renderer/cache.h"
#include "Engine/Renderer/pipeline.h"
#include "Engine/Renderer/descmgr.h"
#include "Engine/Renderer/staging_buffer.h"
#include "Engine/Renderer/matinst.h"

namespace glex
{
	namespace render
	{
		struct FrameResource
		{
			gl::CommandBuffer commandBuffer;
			gl::Semaphore imageAvailableSemaphore;
			gl::Semaphore renderFinishedSemaphore;
			gl::Fence inFlightFence;
			DynamicStagingBuffer stagingBuffer;
			Deque<Function<void()>> deletionQueue;
			FrameResource();
			~FrameResource();
			FrameResource(FrameResource&& rhs);
		};
	}

	struct RenderSettings
	{
		uint8_t renderAheadCount = 2;
		bool enableVsync = true;
		bool useTripleBuffering = true;
	};

	struct RendererStartupInfo
	{
		Function<uint32_t(SequenceView<PhysicalDevice const>)> cardSelector;
		RenderSettings settings;
		uint32_t quadBudget = 2048;
		Pipeline* pipeline = nullptr;
	};

	class Renderer : private StaticClass
	{
	public:
		constexpr static uint32_t GLOBAL_DESCRIPTOR_SET = 0;
		constexpr static uint32_t MATERIAL_DESCRIPOR_SET = 1;
		constexpr static uint32_t OBJECT_DESCRIPTOR_SET = 2;

	private:
		inline static RenderSettings s_renderSettings;
		
		// Several caches.
		inline static render::ShaderModuleCache s_shaderModuleCache;
		inline static render::DescriptorLayoutCache s_descriptorLayoutCache;
		inline static render::PipelineStateCache s_pipelineStateCache;
		inline static Optional<render::StaticDescriptorAllocator> s_staticMaterialDescriptorAllocator;
		// Current state.
		inline static WeakPtr<MaterialInstance> s_currentMaterialInstance;
		// Frame resources.
		inline static Vector<render::FrameResource> s_frameResources;
		inline static uint32_t s_currentFrame;		
		// Staging buffer.
		inline static void* s_stagingBufferData;
		inline static SharedPtr<Buffer> s_stagingBuffer;
		inline static gl::Fence s_transferFence;
		inline static Pipeline* s_renderPipeline;

	public:
		static void Startup(RendererStartupInfo const& info);
		static void Shutdown();
		static void Tick();
		static void Resize();
		static uint32_t CurrentFrame() { return s_currentFrame; }
		static gl::CommandBuffer CurrentCommandBuffer() { return s_frameResources[s_currentFrame].commandBuffer; }
		static RenderSettings const& GetRenderSettings() { return s_renderSettings; }
		static render::ShaderModuleCache& GetShaderModuleCache() { return s_shaderModuleCache; }
		static render::DescriptorLayoutCache& GetDescriptorLayoutCache() { return s_descriptorLayoutCache; }
		static render::PipelineStateCache& GetPipelineStateCache() { return s_pipelineStateCache; }
		static gl::DescriptorSet AllocateStaticMaterialDescriptorSet(gl::DescriptorSetLayout layout);
		static void FreeStaticMaterialDescriptorSet(gl::DescriptorSet set);
		static Pipeline* GetRenderPipeline() { return s_renderPipeline; }

		template <typename Fn>
		static void PendingDelete(Fn&& fn)
		{
			s_frameResources[s_currentFrame].deletionQueue.emplace_back(std::forward<Fn>(fn));
		}

		static void AutomaticLayoutTransition(gl::CommandBuffer commandBuffer, WeakPtr<Image> image, gl::ImageAspect aspect, uint32_t layer, uint32_t numLayers, gl::ImageLayout layoutBefore, gl::ImageLayout layoutAfter);
		static void UploadBuffer(WeakPtr<Buffer> buffer, uint32_t offset, uint32_t size, void const* data);
		static bool UploadImage(WeakPtr<Image> image, uint32_t layer, glm::uvec2 size, uint32_t sizePerPixel, void const* data);
		static bool UploadBufferDynamic(WeakPtr<Buffer> buffer, uint32_t offset, uint32_t size, void const* data, gl::PipelineStage waitStage, gl::Access waitAccess, gl::PipelineStage stageAfter, gl::Access accessAfter);
		static WeakPtr<MaterialInstance>& GetCurrentMaterialInstance() { return s_currentMaterialInstance; }
	};
}