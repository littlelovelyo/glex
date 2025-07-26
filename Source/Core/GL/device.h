#pragma once
#include "Core/Container/basic.h"
#include <vulkan/vulkan.h>

namespace glex
{
	void operator!(VkResult result);

	struct PhysicalDevice
	{
		VkPhysicalDevice handle;
		String name;
		uint32_t vendorID, deviceID;
		uint64_t dedicatedMemory;
		uint64_t sharedMemory;
		uint32_t pushConstantsSize;
		uint32_t minWidth, maxWidth, minHeight, maxHeight;
		uint32_t graphicsQueueIndex;
		uint32_t transferQueueIndex;
		uint32_t presentQueueIndex;
		VkColorSpaceKHR colorSpace;
		bool isDedicated;
		bool supportsImmediatePresenting;
		bool supportsTripleBuffering;
		bool supportsWireframeRendering;
		bool supportsWideLineRendering;
		bool supportsAnisotropicSampling;
		uint8_t maxMSAALevel;
		float maxAnisotropyLevel;
		uint32_t maxTextureCount;
		uint32_t maxSamplerCount;

		enum Vendor : uint32_t
		{
			AMD = 0x1002,
			ImgTec = 0x1010,
			NVIDIA = 0x10DE,
			ARM = 0x13B5,
			Qualcomm = 0x5143,
			Intel = 0x8086
		};
	};
}