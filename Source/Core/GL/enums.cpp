#include "Core/GL/enums.h"
#include "Core/log.h"

using namespace glex;
using namespace glex::gl;

static VkFormat s_formatTable[]
{
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_X8_D24_UNORM_PACK32,
	VK_FORMAT_D32_SFLOAT, // May not be supported.
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D32_SFLOAT_S8_UINT
};

// This lookup table is used 99% of the time.
static ImageFormat s_firstFormatTable[]
{
	ImageFormat::R,
	ImageFormat::RG,
	ImageFormat::RGB,
	ImageFormat::RGBA,
	ImageFormat::RGBA16F,
	ImageFormat::Depth16,
	ImageFormat::Depth24,
	ImageFormat::Depth32,
	ImageFormat::Depth24Stencil8,
	ImageFormat::Depth32Stencil8
};

// If the table above doesn't give us a usable format,
// use this linked list to test each format.
constexpr static ImageFormat FORMAT_END = static_cast<ImageFormat>(255);
static ImageFormat s_revertTable[]
{
	FORMAT_END,
	FORMAT_END,
	FORMAT_END,
	FORMAT_END,
	FORMAT_END,
	FORMAT_END,
	ImageFormat::Depth32,
	ImageFormat::Depth16,
	ImageFormat::Depth32Stencil8,
	ImageFormat::Depth24Stencil8
};

static ImageUsage s_usageFlags[]
{
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::ColorAttachment | ImageUsage::TransferDest,
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::ColorAttachment | ImageUsage::TransferDest,
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::ColorAttachment | ImageUsage::TransferDest,
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::ColorAttachment | ImageUsage::TransferDest,
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::ColorAttachment | ImageUsage::TransferDest,
	ImageUsage::SampledTexture | ImageUsage::TransferSource | ImageUsage::DepthStencilAttachment,
	ImageUsage::None,
	ImageUsage::SampledTexture | ImageUsage::TransferSource,
	ImageUsage::None,
	ImageUsage::None
};

ImageUsage VulkanEnum::FillUsageFlags(uint32_t flags)
{
	ImageUsage usage = ImageUsage::None;
	if (flags & VK_FORMAT_FEATURE_BLIT_SRC_BIT)
		usage = usage | ImageUsage::TransferSource;
	if (flags & VK_FORMAT_FEATURE_BLIT_DST_BIT)
		usage = usage | ImageUsage::TransferDest;
	if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
		usage = usage | ImageUsage::SampledTexture;
	if (flags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) // Review this later.
		usage = usage | ImageUsage::Storage;
	if (flags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
		usage = usage | ImageUsage::ColorAttachment;
	if (flags & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		usage = usage | ImageUsage::DepthStencilAttachment;
	return usage;
}

void VulkanEnum::Startup(VkPhysicalDevice device)
{
	constexpr uint32_t DEPTH_FLAGS = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkFormatProperties formatProp;
	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_X8_D24_UNORM_PACK32, &formatProp);
	s_usageFlags[*ImageFormat::Depth24] = FillUsageFlags(formatProp.optimalTilingFeatures);
	bool depth24 = (formatProp.optimalTilingFeatures & DEPTH_FLAGS) == DEPTH_FLAGS;

	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D32_SFLOAT, &formatProp);
	s_usageFlags[*ImageFormat::Depth32] = FillUsageFlags(formatProp.optimalTilingFeatures);
	bool depth32 = (formatProp.optimalTilingFeatures & DEPTH_FLAGS) == DEPTH_FLAGS;

	if (!depth24)
	{
		if (depth32)
		{
			Logger::Info("Depth24 is unusable. Revert to Depth32.");
			s_firstFormatTable[*ImageFormat::Depth24] = ImageFormat::Depth32;
		}
		else
		{
			Logger::Info("Depth24 is unusable. Revert to Depth16.");
			s_firstFormatTable[*ImageFormat::Depth24] = ImageFormat::Depth16;
			Logger::Info("Depth32 is unusable. Revert to Depth16.");
			s_firstFormatTable[*ImageFormat::Depth32] = ImageFormat::Depth16;
		}
	}
	else if (!depth32)
	{
		Logger::Info("Depth32 is unusable. Revert to Depth24.");
		s_firstFormatTable[*ImageFormat::Depth32] = ImageFormat::Depth24;
	}

	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D24_UNORM_S8_UINT, &formatProp);
	s_usageFlags[*ImageFormat::Depth24Stencil8] = FillUsageFlags(formatProp.optimalTilingFeatures);
	bool depth24Stencil8 = formatProp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D32_SFLOAT_S8_UINT, &formatProp);
	s_usageFlags[*ImageFormat::Depth32Stencil8] = FillUsageFlags(formatProp.optimalTilingFeatures);
	bool depth32Stencil8 = formatProp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if (!depth24Stencil8)
	{
		Logger::Info("Depth24Stencil8 is unusable. Revert to Depth32Stencil8.");
		s_firstFormatTable[*ImageFormat::Depth24Stencil8] = ImageFormat::Depth32Stencil8;
	}
	else if (!depth32Stencil8)
	{
		Logger::Info("Depth32Stencil8 is unusable. Revert to Depth24Stencil8.");
		s_firstFormatTable[*ImageFormat::Depth32Stencil8] = ImageFormat::Depth24Stencil8;
	}
}

ImageFormat VulkanEnum::FindSuitableImageFormat(ImageFormat format, ImageUsage usages)
{
	ImageFormat firstMatch = s_firstFormatTable[*format];
	if ((s_usageFlags[*firstMatch] & usages) == usages)
		return firstMatch;
	for (ImageFormat next = s_revertTable[*firstMatch]; next != firstMatch && next != FORMAT_END; next = s_revertTable[*next])
	{
		if ((s_usageFlags[*next] & usages) == usages)
		{
			Logger::Info("Format %d is not suitable for usage: %d. Revert to %d.", *format, *usages, *next);
			return next;
		}
	}
	Logger::Error("Cannot find format %d with usage: %d.", *format, *usages);
	return ImageFormat::Invalid;
}

ImageUsage VulkanEnum::GetFormatCapabilities(ImageFormat format)
{
	return s_usageFlags[*format];
}

VkFormat VulkanEnum::GetImageFormat(ImageFormat format)
{
	return s_formatTable[*format];
}

static VkFormat s_dataFormatTable[]
{
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32A32_UINT
};

static char const* s_dataTypeNameTable[] =
{
	"float",
	"vec2",
	"vec3",
	"vec4",
	"int",
	"ivec2",
	"ivec3",
	"ivec4",
	"uint",
	"uvec2",
	"uvec3",
	"uvec4"
};

VkFormat VulkanEnum::GetDataFormat(DataType type)
{
	return s_dataFormatTable[*type];
}

char const* VulkanEnum::GetDataTypeName(DataType type)
{
	return s_dataTypeNameTable[*type];
}