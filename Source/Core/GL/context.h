/**
 * The Vulkan context.
 * Since this class is already very large so I decided to move some stuff to the Renderer class.
 */
#pragma once
#include "Core/GL/device.h"
#include "Core/Container/sequence.h"
#include "Core/Container/function.h"
#include "Core/Container/optional.h"
#include "Core/GL/frame_buffer.h"
#include "Core/GL/command.h"
#include "Core/GL/sync.h"
#include <vma/vk_mem_alloc.h>

namespace glex
{
	struct ContextStartupInfo
	{
		Function<PhysicalDevice*(SequenceView<PhysicalDevice const>)> cardSelector;
		bool useTripleBuffering = false;
		bool enableVsync = true;
	};

	namespace gl
	{
		class Context : private StaticClass
		{
		private:
			inline static VkAllocationCallbacks s_hostAllocator;
			inline static VkInstance s_instance;
#if GLEX_ENABLE_VALIDATION_LAYER
			inline static VkDebugUtilsMessengerEXT s_debugMessenger;
#endif
			inline static VkSurfaceKHR s_windowSurface;
			inline static VkDevice s_device;
			inline static VkQueue s_graphicsQueue;
			inline static VkQueue s_presentQueue;
			inline static VkQueue s_transferQueue;
			inline static CommandPool s_graphicsCommandPool;
			inline static CommandPool s_transferCommandPool;
			inline static VkSwapchainKHR s_swapChain;
			inline static VkImage s_swapChainImages[3];
			inline static uint32_t s_currentImage;
			inline static glm::uvec2 s_size;
			inline static PhysicalDevice s_deviceInfo;
			inline static VmaAllocator s_memoryAllocator;

			static uint32_t CreateInstance();
			static void SelectCard(Function<PhysicalDevice*(SequenceView<PhysicalDevice const>)> const& cardSelector);
			static Vector<PhysicalDevice> FilterCards(Vector<VkPhysicalDevice> const& cards);
			static void CreateDevice();
			static void CreateSwapChain(bool enableVsync, bool useTripleBuffering);

		public:
			static void Startup(ContextStartupInfo const& info);
			static void Shutdown();
			static void Wait() { vkDeviceWaitIdle(s_device); }
			static PhysicalDevice const& DeviceInfo() { return s_deviceInfo; }
			static uint32_t Width() { return s_size.x; }
			static uint32_t Height() { return s_size.y; }
			static glm::uvec2 Size() { return s_size; }
			static void RecreateSwapchain(bool enableVsync, bool useTripleBuffering);
			static VkDevice GetDevice() { return s_device; }
			static VkAllocationCallbacks* HostAllocator() { return nullptr; }
			static VmaAllocator GetAllocator() { return s_memoryAllocator; }
			static CommandPool GetGraphicsCommandPool() { return s_graphicsCommandPool; }
			static CommandPool GetTransferCommandPool() { return s_transferCommandPool; }
			static VkQueue GetGraphicsQueue() { return s_graphicsQueue; }
			static VkQueue GetTransferQueue() { return s_transferQueue; }
			static void SubmitCommand(VkQueue queue, CommandBuffer commandBuffer, Semaphore waitSemaphore, PipelineStage waitStage, Semaphore signalSemaphore, PipelineStage signalStage, Fence signalFence);
			static void WaitQueue(VkQueue queue);
			static Image AcquireSwapchainImage(Semaphore signalSemaphore);
			static void Present(Semaphore waitSemaphore);
		};
	}
}