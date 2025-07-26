#include "Core/GL/context.h"
#include "Core/assert.h"
#include "Core/GL/enums.h"
#include "Core/Container/basic.h"
#include "Core/Platform/window.h"
#include "Core/Platform/filesync.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_profiles.hpp>
#include <bit>

using namespace glex;
using namespace glex::gl;

#define GLEX_BOOL_ONOFF(x) ((x) ? "ON" : "OFF")
#define GLEX_BOOL_SUPPORTED(x) ((x) ? "Supported" : "Unsupported")

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		VULKAN CONTEXT STARTUP
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
#if GLEX_ENABLE_VALIDATION_LAYER
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Logger::Warn(pCallbackData->pMessage);
		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Logger::Error(pCallbackData->pMessage);
		break;
		default:
		Logger::Info(pCallbackData->pMessage);
	}
	return VK_FALSE;
}
#endif

uint32_t Context::CreateInstance()
{
	uint32_t vulkanVersion;
	!vkEnumerateInstanceVersion(&vulkanVersion);
	if (vulkanVersion < VK_VERSION_1_3)
		Logger::Fatal("glex requires Vulkan 1.3 to run.");

	/*VpProfileProperties profile { VP_LUNARG_minimum_requirements_1_3, VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION };
	VkBool32 profileSupported;
	!vpGetInstanceProfileSupport(nullptr, &profile, &profileSupported);
	GLEX_ASSERT_MSG(profileSupported, "VP_LUNARG_minimun_requirements_1_3 is not supported.") {}*/

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "glex application";
	appInfo.pEngineName = "glex";
	appInfo.apiVersion = vulkanVersion;
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	// Validation layer.
#if GLEX_ENABLE_VALIDATION_LAYER
	char const* extensions[2] = { "VK_KHR_surface", "VK_EXT_debug_utils" };
	char const* validationLayerName = "VK_LAYER_KHRONOS_validation";
	bool validationAvailable = false;
	uint32_t numLayers;
	!vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
	Vector<VkLayerProperties> layers(numLayers);
	!vkEnumerateInstanceLayerProperties(&numLayers, layers.data());
	for (VkLayerProperties& layer : layers)
	{
		if (!strcmp(layer.layerName, validationLayerName))
		{
			validationAvailable = true;
			break;
		}
	}
	if (validationAvailable)
	{
		instanceInfo.enabledLayerCount = 1;
		instanceInfo.ppEnabledLayerNames = &validationLayerName;
		instanceInfo.enabledExtensionCount = 2;

	}
	else
	{
		Logger::Warn("Vulkan validation layer is not available.");
		instanceInfo.enabledExtensionCount = 1;
	}
#else
	instanceInfo.enabledExtensionCount = 1;
#endif
	instanceInfo.ppEnabledExtensionNames = extensions;

#if GLEX_ENABLE_VALIDATION_LAYER
	VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
	if (validationAvailable)
	{
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = DebugCallback;
		instanceInfo.pNext = &debugInfo;
	}
#endif
	!vkCreateInstance(&instanceInfo, HostAllocator(), &s_instance);

#if GLEX_ENABLE_VALIDATION_LAYER
	if (validationAvailable)
	{
		auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(s_instance, "vkCreateDebugUtilsMessengerEXT"));
		GLEX_ASSERT(createDebugMessenger != nullptr) {}
		!createDebugMessenger(s_instance, &debugInfo, nullptr, &s_debugMessenger);
	}
#endif
	return vulkanVersion;
}

void Context::SelectCard(Function<PhysicalDevice*(SequenceView<PhysicalDevice const>)> const& cardSelector)
{
	uint32_t numCards;
	!vkEnumeratePhysicalDevices(s_instance, &numCards, nullptr);
	if (numCards == 0)
		Logger::Fatal("No suitable GPU found on your PC.");
	Vector<VkPhysicalDevice> cards(numCards);
	!vkEnumeratePhysicalDevices(s_instance, &numCards, cards.data());
	Vector<PhysicalDevice> suitableCards = FilterCards(cards);
	if (suitableCards.empty())
		Logger::Fatal("No suitable GPU found on your PC.");
	PhysicalDevice* selectedCard = nullptr;

	// Default card selector: select whatever the first dedicated card we have. If we don't, select the first.
	auto defaultSelector = [&]()
	{
		for (PhysicalDevice& cardInfo : suitableCards)
		{
			if (cardInfo.isDedicated)
				return &cardInfo;
		}
		return &suitableCards[0];
	};

	if (cardSelector == nullptr)
		selectedCard = defaultSelector();
	else
	{
		selectedCard = cardSelector(suitableCards);
		if (selectedCard == nullptr)
			Logger::Fatal("No graphics card selected.");
	}
	s_deviceInfo = std::move(*selectedCard);
}

void Context::CreateDevice()
{
	// Create logical device.
	HashSet<uint32_t> queueFamilyIndices;
	queueFamilyIndices.insert(s_deviceInfo.graphicsQueueIndex);
	queueFamilyIndices.insert(s_deviceInfo.transferQueueIndex);
	queueFamilyIndices.insert(s_deviceInfo.presentQueueIndex);
	Vector<VkDeviceQueueCreateInfo> queueInfo(queueFamilyIndices.size());
	uint32_t i = 0;
	float const PRIORITY = 1.0f;
	for (uint32_t queueFamily : queueFamilyIndices)
	{
		queueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[i].queueFamilyIndex = queueFamily;
		queueInfo[i].queueCount = 1;
		queueInfo[i].pQueuePriorities = &PRIORITY;
		i++;
	}

	// If it supports the feature, just turn it on, so we don't have to recreate the device over and over again.
	char const* const SWAP_CHAIN_NAME = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	VkPhysicalDeviceSynchronization2Features sync2Feature = {};
	sync2Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2Feature.synchronization2 = VK_TRUE;
	VkPhysicalDeviceFeatures features = {};
	features.fillModeNonSolid = s_deviceInfo.supportsWireframeRendering;
	features.wideLines = s_deviceInfo.supportsWideLineRendering;
	features.samplerAnisotropy = s_deviceInfo.supportsAnisotropicSampling;
	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = &sync2Feature;
	deviceInfo.queueCreateInfoCount = queueInfo.size();
	deviceInfo.pQueueCreateInfos = queueInfo.data();
	deviceInfo.enabledExtensionCount = 1;
	deviceInfo.ppEnabledExtensionNames = &SWAP_CHAIN_NAME;
	deviceInfo.pEnabledFeatures = &features;
	!vkCreateDevice(s_deviceInfo.handle, &deviceInfo, HostAllocator(), &s_device);
	vkGetDeviceQueue(s_device, s_deviceInfo.graphicsQueueIndex, 0, &s_graphicsQueue);
	vkGetDeviceQueue(s_device, s_deviceInfo.transferQueueIndex, 0, &s_transferQueue);
	vkGetDeviceQueue(s_device, s_deviceInfo.presentQueueIndex, 0, &s_presentQueue);
}

void Context::Startup(ContextStartupInfo const& info)
{
	uint32_t vulkanVersion = CreateInstance();
	s_windowSurface = Window::CreateSurface(s_instance);
	SelectCard(info.cardSelector);
	gl::VulkanEnum::Startup(s_deviceInfo.handle);
	CreateDevice();

	// Create memory allocator.
	VmaAllocatorCreateInfo vmaInfo = {};
	vmaInfo.physicalDevice = s_deviceInfo.handle;
	vmaInfo.device = s_device;
	vmaInfo.instance = s_instance;
	vmaInfo.vulkanApiVersion = vulkanVersion;
	!vmaCreateAllocator(&vmaInfo, &s_memoryAllocator);

	// Create command pool and command buffers for each frame.
	if (!s_graphicsCommandPool.Create(s_deviceInfo.graphicsQueueIndex, false) ||
		!s_transferCommandPool.Create(s_deviceInfo.transferQueueIndex, true))
		Logger::Fatal("Cannot create command pools.");

	// Create swapchain.
	CreateSwapChain(info.enableVsync, info.useTripleBuffering && s_deviceInfo.supportsTripleBuffering);

	// Log some information.
	Logger::Info(R"~(+----------------------------------+
|        DEVICE INFORMATION        |
+----------------------------------+
| Vulkan version: %d.%d.%d
| GPU: %s
| Dedicated memory: %f GB
| Shared memory: %f GB
| Triple buffering: %s
| Wireframe rendering: %s
| Wide line rendering: %s
| Max MSAA: %dx
| Max textures: %d
| Max push constants: %d
+-----------------------------------)~",
VK_API_VERSION_MAJOR(vulkanVersion), VK_API_VERSION_MINOR(vulkanVersion), VK_API_VERSION_PATCH(vulkanVersion),
s_deviceInfo.name.c_str(),
static_cast<float>(s_deviceInfo.dedicatedMemory) / Limits::GB,
static_cast<float>(s_deviceInfo.sharedMemory) / Limits::GB,
GLEX_BOOL_SUPPORTED(s_deviceInfo.supportsTripleBuffering),
GLEX_BOOL_SUPPORTED(s_deviceInfo.supportsWireframeRendering),
GLEX_BOOL_SUPPORTED(s_deviceInfo.supportsWideLineRendering),
s_deviceInfo.maxMSAALevel,
s_deviceInfo.maxTextureCount,
s_deviceInfo.pushConstantsSize);
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		CARD FILTER
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
Vector<PhysicalDevice> Context::FilterCards(Vector<VkPhysicalDevice> const& cards)
{
	Vector<PhysicalDevice> result;
	result.reserve(cards.size());
	for (VkPhysicalDevice device : cards)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		if (properties.apiVersion < VK_VERSION_1_3)
			continue;

		/*
		VpProfileProperties profile { VP_LUNARG_minimum_requirements_1_3, VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION };
		VkBool32 profileSupported;
		if (vpGetPhysicalDeviceProfileSupport(s_instance, device, &profile, &profileSupported) != VK_SUCCESS || !profileSupported)
			return false;
		*/

		////// Check synchronization 2 support. //////
		VkPhysicalDeviceSynchronization2Features sync2Feature = {};
		sync2Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		VkPhysicalDeviceFeatures2 features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &sync2Feature;
		vkGetPhysicalDeviceFeatures2(device, &features);
		if (!sync2Feature.synchronization2)
			continue;

		////// Check swap chain availability. //////
		uint32_t numExtensions;
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr) != VK_SUCCESS)
			continue;
		Vector<VkExtensionProperties> extensions(numExtensions);
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, extensions.data()) != VK_SUCCESS)
			continue;
		for (VkExtensionProperties& extension : extensions)
		{
			if (strcmp(extension.extensionName, "VK_KHR_swapchain") == 0)
				goto SWAPCHAIN_AVAILABLE;
		}
		continue;
	SWAPCHAIN_AVAILABLE:

		////// Present mode //////
		PhysicalDevice deviceInfo;
		VkSurfaceCapabilitiesKHR capabilities;
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, s_windowSurface, &capabilities) != VK_SUCCESS)
			continue;
		if (capabilities.minImageCount > 2 || capabilities.maxImageCount != 0 && capabilities.maxImageCount < 2)
			continue; // It doesn't even support double buffering!
		uint32_t numPresentModes;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, s_windowSurface, &numPresentModes, nullptr) != VK_SUCCESS)
			continue;
		Vector<VkPresentModeKHR> presentModes(numPresentModes);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, s_windowSurface, &numPresentModes, presentModes.data()) != VK_SUCCESS)
			continue;
		deviceInfo.supportsImmediatePresenting = false;
		deviceInfo.supportsTripleBuffering = false;
		for (VkPresentModeKHR presentMode : presentModes)
		{
			// FIFO is mandatory. These two are probably not.
			if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				deviceInfo.supportsImmediatePresenting = true;
				if (deviceInfo.supportsTripleBuffering)
					break;
			}
			else if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				deviceInfo.supportsTripleBuffering = true;
				if (deviceInfo.supportsImmediatePresenting)
					break;
			}
		}
		deviceInfo.supportsTripleBuffering = deviceInfo.supportsTripleBuffering && (capabilities.maxImageCount == 0 || capabilities.maxImageCount >= 3);;

		////// Format //////
		uint32_t numFormats;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, s_windowSurface, &numFormats, nullptr) != VK_SUCCESS)
			continue;
		Vector<VkSurfaceFormatKHR> formats(numFormats);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, s_windowSurface, &numFormats, formats.data()) != VK_SUCCESS)
			continue;
		for (VkSurfaceFormatKHR& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM) // This format and BLIT_DEST flag is mandatory. So it's certainly safe.
			{
				deviceInfo.colorSpace = format.colorSpace; // Return whatever color space it is.
				goto FORMAT_PASS;
			}
		}
		continue;
	FORMAT_PASS:

		////// Allocate queue index. //////
		uint32_t numQueueFamily;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamily, nullptr);
		Vector<VkQueueFamilyProperties> queueFamilies(numQueueFamily);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamily, queueFamilies.data());
		deviceInfo.graphicsQueueIndex = UINT_MAX;
		deviceInfo.transferQueueIndex = UINT_MAX;
		deviceInfo.presentQueueIndex = UINT_MAX;
		uint32_t transferWeight = 33;
		for (uint32_t i = 0; i < queueFamilies.size(); i++)
		{
			VkQueueFamilyProperties& queue = queueFamilies[i];
			bool graphics = queue.queueFlags & VK_QUEUE_GRAPHICS_BIT;
			bool transfer = queue.queueFlags & VK_QUEUE_TRANSFER_BIT;
			uint32_t weight = std::popcount(queue.queueFlags);
			VkBool32 present;
			if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, s_windowSurface, &present) != VK_SUCCESS)
				goto SKIP_THIS_CARD;
			if (graphics && deviceInfo.graphicsQueueIndex == UINT_MAX)
				deviceInfo.graphicsQueueIndex = i;
			if (present && deviceInfo.presentQueueIndex == UINT_MAX)
				deviceInfo.presentQueueIndex = i;
			// Use a dedicated tranfer queue.
			// Some GPUs may only have one queue family (i.e. Intel HD Graphics 630).
			// 
			// The logic is:
			// 1. We don't have a transfer queue yet;
			// 2. Transfer queue is the same as graphics queue;
			// 2. Transfer queue is more dedicated.
			if (transfer && (deviceInfo.transferQueueIndex == UINT_MAX ||
				deviceInfo.transferQueueIndex == deviceInfo.graphicsQueueIndex ||
				weight < transferWeight /* && i != outMoreInfo.graphicsQueueIndex */))
			{
				deviceInfo.transferQueueIndex = i;
				transferWeight = weight;
			}
		}
		if (deviceInfo.graphicsQueueIndex != UINT_MAX && deviceInfo.transferQueueIndex != UINT_MAX && deviceInfo.presentQueueIndex != UINT_MAX)
		{
			////// Non-mandatory properties //////
			deviceInfo.handle = device;
			deviceInfo.name = properties.deviceName;
			deviceInfo.vendorID = properties.vendorID;
			deviceInfo.deviceID = properties.deviceID;
			deviceInfo.isDedicated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			deviceInfo.pushConstantsSize = properties.limits.maxPushConstantsSize;
			deviceInfo.minWidth = capabilities.minImageExtent.width;
			deviceInfo.maxWidth = capabilities.maxImageExtent.width;
			deviceInfo.minHeight = capabilities.minImageExtent.height;
			deviceInfo.maxHeight = capabilities.maxImageExtent.height;
			deviceInfo.supportsWireframeRendering = features.features.fillModeNonSolid;
			deviceInfo.supportsWideLineRendering = features.features.wideLines;
			deviceInfo.supportsAnisotropicSampling = features.features.samplerAnisotropy;
			deviceInfo.maxMSAALevel = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
			deviceInfo.maxAnisotropyLevel = properties.limits.maxSamplerAnisotropy;
			deviceInfo.maxTextureCount = properties.limits.maxPerStageDescriptorSampledImages;
			deviceInfo.maxSamplerCount = properties.limits.maxPerStageDescriptorSamplers;
			VkPhysicalDeviceMemoryProperties memoryProperties;
			vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
			deviceInfo.dedicatedMemory = 0;
			deviceInfo.sharedMemory = 0;
			for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
			{
				VkMemoryHeap& heap = memoryProperties.memoryHeaps[i];
				if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					deviceInfo.dedicatedMemory += heap.size;
				else
					deviceInfo.sharedMemory += heap.size;
			}
			result.push_back(deviceInfo);
		}
	SKIP_THIS_CARD:;
	}
	return result;
}
void Context::Shutdown()
{
	vkDestroySwapchainKHR(s_device, s_swapChain, nullptr);
#if GLEX_REPORT_MEMORY_LEAKS
	s_deviceInfo.name.clear();
	s_deviceInfo.name.shrink_to_fit();
#endif
	vmaDestroyAllocator(s_memoryAllocator);
	s_transferCommandPool.Destroy();
	s_graphicsCommandPool.Destroy();
	vkDestroyDevice(s_device, nullptr);
	vkDestroySurfaceKHR(s_instance, s_windowSurface, nullptr);
#if GLEX_ENABLE_VALIDATION_LAYER
	auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(s_instance, "vkDestroyDebugUtilsMessengerEXT"));
	GLEX_ASSERT(destroyDebugMessenger != nullptr) {}
	destroyDebugMessenger(s_instance, s_debugMessenger, nullptr);
#endif
	vkDestroyInstance(s_instance, nullptr);
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		SWAPCHAIN CREATION
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
void Context::CreateSwapChain(bool enableVsync, bool useTripleBuffering)
{
	GLEX_DEBUG_ASSERT(!useTripleBuffering || s_deviceInfo.supportsTripleBuffering) {}
	s_size.x = Window::Width(), s_size.y = Window::Height();
	if (s_size.y < s_deviceInfo.minWidth || s_size.y > s_deviceInfo.maxWidth || s_size.y < s_deviceInfo.minHeight || s_size.y > s_deviceInfo.maxHeight)
	{
		// Query it again to be safe.
		VkSurfaceCapabilitiesKHR capabilities;
		!vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_deviceInfo.handle, s_windowSurface, &capabilities);
		s_deviceInfo.minWidth = capabilities.minImageExtent.width;
		s_deviceInfo.maxWidth = capabilities.maxImageExtent.width;
		s_deviceInfo.minHeight = capabilities.minImageExtent.height;
		s_deviceInfo.maxHeight = capabilities.maxImageExtent.height;
		s_size.x = glm::clamp(s_size.x, s_deviceInfo.minWidth, s_deviceInfo.maxWidth);
		s_size.y = glm::clamp(s_size.y, s_deviceInfo.minHeight, s_deviceInfo.maxHeight);
	}

	// Create swap chain.
	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.surface = s_windowSurface;
	swapChainInfo.minImageCount = useTripleBuffering ? 3 : 2;
	swapChainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapChainInfo.imageColorSpace = s_deviceInfo.colorSpace;
	swapChainInfo.imageExtent = { s_size.x, s_size.y };
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.presentMode = enableVsync ? (useTripleBuffering ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR) : VK_PRESENT_MODE_IMMEDIATE_KHR;
	swapChainInfo.clipped = VK_TRUE;
	!vkCreateSwapchainKHR(s_device, &swapChainInfo, nullptr, &s_swapChain);

	// Get images.
	uint32_t numImages;
	!vkGetSwapchainImagesKHR(s_device, s_swapChain, &numImages, nullptr);
	GLEX_ASSERT_MSG(numImages == swapChainInfo.minImageCount, "Cannot create swap chain!") {}
	!vkGetSwapchainImagesKHR(s_device, s_swapChain, &numImages, s_swapChainImages);
}

void Context::RecreateSwapchain(bool enableVsync, bool useTripleBuffering)
{
	vkDeviceWaitIdle(s_device);
	vkDestroySwapchainKHR(s_device, s_swapChain, HostAllocator());
	// vkDestroySurfaceKHR(s_instance, s_windowSurface, nullptr);
	// !glfwCreateWindowSurface(s_instance, Window::GetHandle(), nullptr, &s_windowSurface);
	CreateSwapChain(enableVsync, useTripleBuffering);
}

Image Context::AcquireSwapchainImage(Semaphore signalSemaphore)
{
	// This fails when showing desktop. Skip the frame when this happens.
	VkResult ret = vkAcquireNextImageKHR(s_device, s_swapChain, UINT64_MAX, signalSemaphore.GetHandle(), VK_NULL_HANDLE, &s_currentImage);
	if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR)
		Logger::Fatal("vkAcquireNextImageKHR failed because of %d.", ret);
	return s_swapChainImages[s_currentImage];
}

void Context::SubmitCommand(VkQueue queue, CommandBuffer commandBuffer, Semaphore waitSemaphore, PipelineStage waitStage, Semaphore signalSemaphore, PipelineStage signalStage, Fence signalFence)
{
	VkSemaphoreSubmitInfo waitInfo;
	VkSemaphoreSubmitInfo signalInfo;
	VkSubmitInfo2 submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	if (waitSemaphore.GetHandle() != VK_NULL_HANDLE)
	{
		waitInfo = {};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = waitSemaphore.GetHandle();
		waitInfo.value = 1;
		waitInfo.stageMask = static_cast<VkPipelineStageFlags2>(waitStage);
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
	}
	VkCommandBufferSubmitInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	bufferInfo.commandBuffer = commandBuffer.GetHandle();
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &bufferInfo;
	if (signalSemaphore.GetHandle() != VK_NULL_HANDLE)
	{
		signalInfo = {};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = signalSemaphore.GetHandle();
		signalInfo.value = 1;
		signalInfo.stageMask = static_cast<VkPipelineStageFlags2>(signalStage);
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
	}
	!vkQueueSubmit2(queue, 1, &submitInfo, signalFence.GetHandle());
}

void Context::WaitQueue(VkQueue queue)
{
	!vkQueueWaitIdle(queue);
}

void Context::Present(Semaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = reinterpret_cast<VkSemaphore*>(&waitSemaphore);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &s_swapChain;
	presentInfo.pImageIndices = &s_currentImage;
	VkResult ret = vkQueuePresentKHR(s_presentQueue, &presentInfo);
	if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR)
		Logger::Fatal("vkQueuePresentKHR failed because of %d.", ret);
}