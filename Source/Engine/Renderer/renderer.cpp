#include "Engine/Renderer/renderer.h"
#include "Engine/GUI/batch.h"
#include "Core/GL/context.h"
#include "game.h"
#include <stb/stb_image.h>

using namespace glex;
using namespace glex::gl;
using namespace glex::render;

FrameResource::FrameResource() : stagingBuffer(1_mib)
{
	if ((commandBuffer = Context::GetGraphicsCommandPool().AllocateCommandBuffer()).GetHandle() == VK_NULL_HANDLE ||
		!imageAvailableSemaphore.Create() ||
		!renderFinishedSemaphore.Create() ||
		!inFlightFence.Create(true))
		Logger::Fatal("Cannot create frame resources!");
}

FrameResource::~FrameResource()
{
	if (inFlightFence.GetHandle() != VK_NULL_HANDLE)
	{
		Context::GetGraphicsCommandPool().FreeCommandBuffer(commandBuffer);
		imageAvailableSemaphore.Destroy();
		renderFinishedSemaphore.Destroy();
		inFlightFence.Destroy();
	}
}

FrameResource::FrameResource(FrameResource&& rhs) : commandBuffer(rhs.commandBuffer), imageAvailableSemaphore(rhs.imageAvailableSemaphore),
renderFinishedSemaphore(rhs.renderFinishedSemaphore), inFlightFence(rhs.inFlightFence), deletionQueue(std::move(rhs.deletionQueue)), stagingBuffer(std::move(rhs.stagingBuffer))
{
	rhs.inFlightFence = VK_NULL_HANDLE;
}

void Renderer::Startup(RendererStartupInfo const& info)
{
	// Context startup.
	// TODO: validate render settings.
	ContextStartupInfo contextInfo;
	contextInfo.cardSelector = info.cardSelector;
	contextInfo.useTripleBuffering = info.settings.useTripleBuffering;
	contextInfo.enableVsync = info.settings.enableVsync;
	Context::Startup(contextInfo);
	s_renderSettings = info.settings;
	s_renderSettings.useTripleBuffering = s_renderSettings.useTripleBuffering && Context::DeviceInfo().SupportsTripleBuffering();
	if (info.settings.useTripleBuffering && !s_renderSettings.useTripleBuffering)
		Logger::Warn("Triple buffering is not supported.");

	// Descriptor allocators startup.
	// TODO: change to SequenceView.
	s_staticMaterialDescriptorAllocator.Emplace(std::initializer_list<std::pair<gl::DescriptorType, uint32_t>> { { gl::DescriptorType::UniformBuffer, 1 }, { gl::DescriptorType::CombinedImageSampler, 3 } }, 256);

	// Ourself startup.
	s_frameResources.resize(s_renderSettings.renderAheadCount);
	s_currentFrame = 0;
	constexpr uint32_t intialStagingBufferSize = Limits::TEXTURE_SIZE * Limits::TEXTURE_SIZE * 4; // 256 MB.
	s_stagingBuffer = MakeShared<Buffer>(gl::BufferUsage::TransferSource, intialStagingBufferSize, true);
	if (!s_stagingBuffer->IsValid())
		Logger::Fatal("Cannot create staging buffer. Is shared VRAM too small?");
	if (!s_transferFence.Create(false))
		Logger::Fatal("Cannot create transfer fence.");
	s_stagingBufferData = s_stagingBuffer->GetMemoryObject().Map(0, intialStagingBufferSize);

	// GUI.
	if (!ui::BatchRenderer::Startup(info.quadBudget))
		Logger::Fatal("Cannot startup batch renderer.");

	s_renderPipeline = info.pipeline;

	// Give pipeline a change to run commands.
	//FrameResource& frame = s_frameResources[s_currentFrame];
	//frame.commandBuffer.Reset();
	//frame.commandBuffer.Begin();
	if (s_renderPipeline == nullptr || !s_renderPipeline->Startup())
		Logger::Fatal("Cannot startup render pipeline.");
	//frame.commandBuffer.End();
	//Context::SubmitCommand(Context::GetGraphicsQueue(), frame.commandBuffer, gl::Semaphore(), gl::PipelineStage::None, gl::Semaphore(), gl::PipelineStage::None, gl::Fence());
	//Context::WaitQueue(Context::GetGraphicsQueue());
}

void Renderer::Shutdown()
{
	Context::Wait();
	s_renderPipeline->Shutdown();
	s_renderPipeline->BaseShutdown();
	s_stagingBuffer->GetMemoryObject().Unmap();
	s_transferFence.Destroy();
	s_stagingBuffer = nullptr;
	s_staticMaterialDescriptorAllocator.Destroy();
	ui::BatchRenderer::Shutdown();
	for (FrameResource const& frameResource : s_frameResources)
	{
		for (auto& fn : frameResource.deletionQueue)
			fn();
	}
	s_frameResources.clear();
#if GLEX_REPORT_MEMORY_LEAKS
	s_frameResources.shrink_to_fit();
	s_pipelineStateCache.FreeMemory();
	s_shaderModuleCache.FreeMemory();
	s_descriptorLayoutCache.FreeMemory();
	RenderPass::FreeMemory();
#endif
	Context::Shutdown();
}

gl::DescriptorSet Renderer::AllocateStaticMaterialDescriptorSet(gl::DescriptorSetLayout layout)
{
	return s_staticMaterialDescriptorAllocator->AllocateDescriptorSet(layout);
}

void Renderer::FreeStaticMaterialDescriptorSet(gl::DescriptorSet set)
{
	return s_staticMaterialDescriptorAllocator->FreeDescriptorSet(set);
}

void Renderer::Tick()
{
	FrameResource& frame = s_frameResources[s_currentFrame];
	frame.inFlightFence.Wait();
	frame.inFlightFence.Reset();
	for (auto& fn : frame.deletionQueue)
		fn();
	frame.deletionQueue.clear();
	frame.stagingBuffer.Reset();
	gl::Image swapChainImage = Context::AcquireSwapchainImage(frame.imageAvailableSemaphore);

	// Reset state.
	s_currentMaterialInstance = nullptr;

	ui::BatchRenderer::Tick();

	// Do nothing if no images are available.
	frame.commandBuffer.Reset();
	frame.commandBuffer.Begin();
	WeakPtr<ImageView> renderResult = s_renderPipeline->Render(GameInstance::GetCurrentScene());
	WeakPtr<Image> sourceImage = renderResult->GetImage();
	WeakPtr<Image> resultImage = renderResult->GetImage();
	frame.commandBuffer.ImageMemoryBarrier(swapChainImage, 0, 1, gl::ImageAspect::Color, gl::PipelineStage::None, gl::Access::None, gl::ImageLayout::Undefined, gl::PipelineStage::Blit, gl::Access::TransferWrite, gl::ImageLayout::TransferDest);
	gl::ImageLayout layout = resultImage->GetImageLayout(renderResult->LayerIndex());
	if (layout != gl::ImageLayout::TransferSource)
	{
		Renderer::AutomaticLayoutTransition(frame.commandBuffer, resultImage, gl::ImageAspect::Color, renderResult->LayerIndex(), 1, layout, gl::ImageLayout::TransferSource);
		resultImage->SetImageLayout(renderResult->LayerIndex(), 1, gl::ImageLayout::TransferSource);
	}
	frame.commandBuffer.BlitImage(sourceImage->GetImageObject(), swapChainImage, sourceImage->Size(), Context::Size(), gl::ImageFilter::Nearest);
	frame.commandBuffer.ImageMemoryBarrier(swapChainImage, 0, 1, gl::ImageAspect::Color, gl::PipelineStage::Blit, gl::Access::TransferWrite, gl::ImageLayout::TransferDest, gl::PipelineStage::None, gl::Access::None, gl::ImageLayout::ReadyToPresent);
	frame.commandBuffer.End();
	Context::SubmitCommand(Context::GetGraphicsQueue(), frame.commandBuffer, frame.imageAvailableSemaphore, gl::PipelineStage::All, frame.renderFinishedSemaphore, gl::PipelineStage::All, frame.inFlightFence);
	Context::Present(frame.renderFinishedSemaphore);
	s_currentFrame = (s_currentFrame + 1) % s_frameResources.size();
}

void Renderer::Resize()
{
	Context::RecreateSwapchain(s_renderSettings.enableVsync, s_renderSettings.useTripleBuffering);
	//gl::CommandBuffer commandBuffer = s_frameResources[s_currentFrame].commandBuffer;
	//commandBuffer.Reset();
	//commandBuffer.Begin();
	if (!s_renderPipeline->Resize(Context::Width(), Context::Height()))
		Logger::Fatal("Pipeline resize failed!");
	//commandBuffer.End();
	//Context::SubmitCommand(Context::GetGraphicsQueue(), commandBuffer, gl::Semaphore(), gl::PipelineStage::None, gl::Semaphore(), gl::PipelineStage::None, gl::Fence());
	//Context::WaitQueue(Context::GetGraphicsQueue());
}

void Renderer::AutomaticLayoutTransition(gl::CommandBuffer commandBuffer, WeakPtr<Image> image, gl::ImageAspect aspect, uint32_t layer, uint32_t numLayers, gl::ImageLayout layoutBefore, gl::ImageLayout layoutAfter)
{
	// Layout transition.
	gl::PipelineStage stageBefore;
	gl::Access accessBefore;
	gl::PipelineStage stageAfter;
	gl::Access accessAfter;
	switch (layoutBefore)
	{
		case gl::ImageLayout::ColorAttachment:
		{
			stageBefore = gl::PipelineStage::ColorOutput;
			accessBefore = gl::Access::ColorWrite;
			break;
		}
		case gl::ImageLayout::DepthStencilAttachment:
		{
			stageBefore = gl::PipelineStage::DepthStencilOutput;
			accessBefore = gl::Access::DepthStencilWrite;
			break;
		}
		case gl::ImageLayout::DepthStencilRead:
		{
			stageBefore = gl::PipelineStage::FragmentShader;
			accessBefore = gl::Access::DepthStencilRead;
			break;
		}
		case gl::ImageLayout::ShaderRead:
		{
			stageBefore = gl::PipelineStage::FragmentShader;
			accessBefore = gl::Access::ShaderSampledRead;
			break;
		}
		case gl::ImageLayout::TransferSource:
		{
			stageBefore = gl::PipelineStage::Copy | gl::PipelineStage::Blit;
			accessBefore = gl::Access::TransferRead;
			break;
		}
		case gl::ImageLayout::TransferDest:
		{
			stageBefore = gl::PipelineStage::Copy | gl::PipelineStage::Blit;
			accessBefore = gl::Access::TransferWrite;
			break;
		}
		default:
		{
			if (layoutBefore != gl::ImageLayout::Undefined)
				Logger::Error("Attachment shouldn't have this initial layout.");
			stageBefore = gl::PipelineStage::None;
			accessBefore = gl::Access::None;
			break;
		}
	}
	switch (layoutAfter)
	{
		case gl::ImageLayout::ColorAttachment:
		{
			layoutBefore = gl::ImageLayout::Undefined;
			stageAfter = gl::PipelineStage::ColorOutput;
			accessAfter = gl::Access::ColorWrite;
			break;
		}
		case gl::ImageLayout::DepthStencilAttachment:
		{
			layoutBefore = gl::ImageLayout::Undefined;
			stageAfter = gl::PipelineStage::DepthStencilOutput;
			accessAfter = gl::Access::DepthStencilWrite;
			break;
		}
		case gl::ImageLayout::ShaderRead:
		{
			stageAfter = gl::PipelineStage::FragmentShader;
			accessAfter = gl::Access::ShaderSampledRead;
			break;
		}
		case gl::ImageLayout::DepthStencilRead:
		{
			stageAfter = gl::PipelineStage::FragmentShader;
			accessAfter = gl::Access::DepthStencilRead;
			break;
		}
		case gl::ImageLayout::TransferSource:
		{
			stageAfter = gl::PipelineStage::Copy | gl::PipelineStage::Blit;
			accessAfter = gl::Access::TransferRead;
			break;
		}
		default:
		{
			Logger::Error("Attachment shouldn't have this final layout.");
			layoutBefore = gl::ImageLayout::Undefined;
			stageAfter = gl::PipelineStage::None;
			accessAfter = gl::Access::None;
			break;
		}
	}
	commandBuffer.ImageMemoryBarrier(image->GetImageObject(), layer, numLayers, aspect, stageBefore, accessBefore, layoutBefore, stageAfter, accessAfter, layoutAfter);
}

void Renderer::UploadBuffer(WeakPtr<Buffer> buffer, uint32_t offset, uint32_t size, void const* data)
{
	// External sync is needed.
	gl::CommandPool transferPool = Context::GetTransferCommandPool();
	gl::CommandBuffer commandBuffer = transferPool.AllocateCommandBuffer();
	while (size != 0)
	{
		uint32_t copyedSize = glm::min(size, s_stagingBuffer->Size());
		memcpy(s_stagingBufferData, data, copyedSize);
		size -= copyedSize;
		data = Mem::Offset(data, copyedSize);
		s_stagingBuffer->GetMemoryObject().Flush(0, copyedSize);
		commandBuffer.Reset();
		commandBuffer.Begin();
		commandBuffer.CopyBuffer(s_stagingBuffer->GetBufferObject(), buffer->GetBufferObject(), 0, offset, copyedSize);
		commandBuffer.End();
		Context::SubmitCommand(Context::GetTransferQueue(), commandBuffer, nullptr, gl::PipelineStage::None, nullptr, gl::PipelineStage::None, s_transferFence);
		offset += copyedSize;
		s_transferFence.Wait();
		s_transferFence.Reset();
	}
	transferPool.FreeCommandBuffer(commandBuffer);
}

bool Renderer::UploadImage(WeakPtr<Image> image, uint32_t layer, glm::uvec2 size, uint32_t sizePerPixel, void const* data)
{
	if (size.x > Limits::TEXTURE_SIZE || size.y > Limits::TEXTURE_SIZE)
	{
		Logger::Error("Texture is too large.");
		return false;
	}
	uint32_t totalSize;
	if (sizePerPixel == 4)
	{
		GLEX_DEBUG_ASSERT(VulkanEnum::GetImageFormat(image->Format()) == VK_FORMAT_B8G8R8A8_UNORM) {}
		// Convert RGBA to BGRA.
		uint32_t pixels = size.x * size.y;
		for (uint32_t i = 0; i < pixels; i++)
		{
			union
			{
				uint32_t rgba;
				struct { uint8_t r, g, b, a; };
			} color;
			color.rgba = reinterpret_cast<uint32_t const*>(data)[i];
			std::swap(color.r, color.b);
			reinterpret_cast<uint32_t*>(s_stagingBufferData)[i] = color.rgba;
		}
		totalSize = pixels * 4;
	}
	else if (sizePerPixel == 3)
	{
		GLEX_DEBUG_ASSERT(VulkanEnum::GetImageFormat(image->Format()) == VK_FORMAT_B8G8R8A8_UNORM) {}
		// Convert RGB to BGRA.
		uint32_t pixels = size.x * size.y;
		for (uint32_t i = 0; i < pixels; i++)
		{
			union
			{
				uint32_t rgba;
				struct { uint8_t r, g, b, a; };
			} color;
			color.b = reinterpret_cast<uint8_t const*>(data)[i * 3];
			color.g = reinterpret_cast<uint8_t const*>(data)[i * 3 + 1];
			color.r = reinterpret_cast<uint8_t const*>(data)[i * 3 + 2];
			color.a = 255;
			reinterpret_cast<uint32_t*>(s_stagingBufferData)[i] = color.rgba;
		}
		totalSize = pixels * 4;
	}
	else
	{
		totalSize = size.x * size.y * sizePerPixel;
		memcpy(s_stagingBufferData, data, totalSize);
	}
	s_stagingBuffer->GetMemoryObject().Flush(0, totalSize);

	gl::CommandPool transferPool = Context::GetTransferCommandPool();
	gl::CommandBuffer commandBuffer = transferPool.AllocateCommandBuffer();
	commandBuffer.Reset();
	commandBuffer.Begin();
	commandBuffer.ImageMemoryBarrier(image->GetImageObject(), layer, 1, gl::ImageAspect::Color, gl::PipelineStage::None, gl::Access::None, gl::ImageLayout::Undefined, gl::PipelineStage::Copy, gl::Access::TransferWrite, gl::ImageLayout::TransferDest);
	commandBuffer.CopyImage(s_stagingBuffer->GetBufferObject(), 0, image->GetImageObject(), layer, gl::ImageAspect::Color, size);
	commandBuffer.ImageMemoryBarrier(image->GetImageObject(), layer, 1, gl::ImageAspect::Color, gl::PipelineStage::Copy, gl::Access::TransferWrite, gl::ImageLayout::TransferDest, gl::PipelineStage::FragmentShader, gl::Access::ShaderSampledRead, gl::ImageLayout::ShaderRead);
	commandBuffer.End();
	Context::SubmitCommand(Context::GetTransferQueue(), commandBuffer, nullptr, gl::PipelineStage::None, nullptr, gl::PipelineStage::None, s_transferFence);
	s_transferFence.Wait();
	s_transferFence.Reset();
	transferPool.FreeCommandBuffer(commandBuffer);
	return true;
}

bool Renderer::UploadBufferDynamic(WeakPtr<Buffer> buffer, uint32_t offset, uint32_t size, void const* data, gl::PipelineStage waitStage, gl::Access waitAccess, gl::PipelineStage stageAfter, gl::Access accessAfter)
{
	FrameResource& frame = s_frameResources[s_currentFrame];
	frame.commandBuffer.BufferMemoryBarrier(buffer->GetBufferObject(), offset, size, waitStage, waitAccess, gl::PipelineStage::Copy, gl::Access::TransferWrite);
	bool result = frame.stagingBuffer.UploadBuffer(buffer, offset, size, data);
	if (!result)
	{
		Logger::Error("Cannot upload dynamic buffer.");
		return false;
	}
	frame.commandBuffer.BufferMemoryBarrier(buffer->GetBufferObject(), offset, size, gl::PipelineStage::Copy, gl::Access::TransferWrite, stageAfter, accessAfter);
	return result;
}