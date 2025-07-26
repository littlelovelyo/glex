#include "Core/GL/device.h"
#include "Core/log.h"

using namespace glex;

// -13~5.
void glex::operator!(VkResult result)
{
	static char const* s_errorCode[] = {
	   "VK_ERROR_UNKNOWN",
	   "VK_ERROR_FRAGMENTED_POOL",
	   "VK_ERROR_FORMAT_NOT_SUPPORTED",
	   "VK_ERROR_TOO_MANY_OBJECTS",
	   "VK_ERROR_INCOMPATIBLE_DRIVER",
	   "VK_ERROR_FEATURE_NOT_PRESENT",
	   "VK_ERROR_EXTENSION_NOT_PRESENT",
	   "VK_ERROR_LAYER_NOT_PRESENT",
	   "VK_ERROR_MEMORY_MAP_FAILED",
	   "VK_ERROR_DEVICE_LOST",
	   "VK_ERROR_INITIALIZATION_FAILED",
	   "VK_ERROR_OUT_OF_DEVICE_MEMORY",
	   "VK_ERROR_OUT_OF_HOST_MEMORY",
	   "VK_SUCCESS",
	   "VK_NOT_READY",
	   "VK_TIMEOUT",
	   "VK_EVENT_SET",
	   "VK_EVENT_RESET",
	   "VK_INCOMPLETE"
	};
	if (result != VK_SUCCESS)
	{
		if (result >= -13 && result <= 5)
			Logger::Fatal("Vulkan error! Error code: %s.", s_errorCode[result + 13]);
		else
			Logger::Fatal("Vulkan error! Error code: %d.", result);
	}
}