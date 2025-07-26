/**
 * Our enum — Vulkan enum conversion layer.
 * If a certain format is not supported, use the backup one.
 */
#pragma once
#include "Core/commdefs.h"
#include <vulkan/vulkan.h>

namespace glex::gl
{
	enum class ImageFormat : uint8_t
	{
		R, // R8_UNORM.
		RG, // R8G8_UNORM.
		RGB, // Not widely supported. Use RGBA instead.
		RGBA, // B8G8R8A8_UNORM or R8G8B8A8_UNORM.
		RGBA16F, // R16G16B16A16_SFLOAT.
		Depth16, // D16_UNORM.
		Depth24, // X8_D24_UNORM.
		Depth32, // D32_SFLOAT.
		Depth24Stencil8, // D24_UNORM_S8_UINT
		Depth32Stencil8, // D32_SFLOAT_S8_UINT
		Invalid = 255
	};

	enum class ImageLayout : uint8_t
	{
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		General = VK_IMAGE_LAYOUT_GENERAL,
		ColorAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		DepthStencilAttachment = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DepthStencilRead = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		ShaderRead = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		TransferSource = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		TransferDest = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ReadyToPresent = 0xff
	};

	// Happens to be the same.
	enum class ImageUsage : uint8_t
	{
		None = 0,
		TransferSource = 1,
		TransferDest = 2,
		SampledTexture = 4,
		Storage = 8,
		ColorAttachment = 16,
		DepthStencilAttachment = 32
		// TransientAttachment = 64
	};

	// Happens to be the same.
	enum class ImageFilter : uint8_t
	{
		Nearest = VK_FILTER_NEAREST,
		Linear = VK_FILTER_LINEAR
	};

	// Happens to be the same.
	enum class ImageWrap : uint8_t
	{
		Repeat,
		Mirrored,
		Clamp,
		Border
	};

	// We should only use one of them unless we're using multi-planar image format.
	enum class ImageAspect : uint8_t
	{
		Color = 1,
		Depth = 2,
		Stencil = 4,
		DepthStencil = 6
	};

	enum class ImageType : uint8_t
	{
		Sampler2D = 1,
		SamplerCube = 3
	};

	enum class DataType : uint8_t
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		Int,
		IVec2,
		IVec3,
		IVec4,
		UInt,
		UVec2,
		UVec3,
		UVec4
	};

	enum class CullMode : uint8_t
	{
		Neither = 0,
		Front = 1,
		Back = 2,
		Both = 3
	};

	// Happens to be the same.
	enum class BlendFactor : uint8_t
	{
		Zero = VK_BLEND_FACTOR_ZERO,
		One = VK_BLEND_FACTOR_ONE,
		SourceColor = VK_BLEND_FACTOR_SRC_COLOR,
		OneMinusSourceColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		DestColor = VK_BLEND_FACTOR_DST_COLOR,
		OneMinusDestColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		SourceAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
		OneMinusSourceAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		DestAlpha = VK_BLEND_FACTOR_DST_ALPHA,
		OneMinusDestAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
	};

	enum class BlendOperation : uint8_t
	{
		Add = VK_BLEND_OP_ADD,
		Subtract = VK_BLEND_OP_SUBTRACT,
		ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
		Min = VK_BLEND_OP_MIN,
		Max = VK_BLEND_OP_MAX
	};

	enum class DescriptorType : uint32_t
	{
		Sampler,
		CombinedImageSampler,
		SampledImage,
		UniformBuffer = 6,
	};

	enum class ShaderStage : uint8_t
	{
		None = 0,
		Vertex = 1,
		Geometry = 8,
		Fragment = 16,
		AllGraphics = 31,
		Compute = 32,
		// All = 63 // Not the same.
	};

	enum class BufferUsage : uint8_t
	{
		Vertex = 128,
		Index = 64,
		Uniform = 16,
		TransferSource = 1,
		TransferDest = 2
	};

	enum class AttachmentLoadOperation : uint8_t
	{
		Load,
		Clear,
		Discard
	};

	enum class AttachmentStoreOperation : uint8_t
	{
		Store,
		Discard
	};

	enum class PipelineStage : VkPipelineStageFlags2
	{
		None = VK_PIPELINE_STAGE_2_NONE,
		All = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		AllGraphics = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		VertexShader = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
		FragmentShader = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		DepthStencilOutput = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
		ColorOutput = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		// Transfer stages are parallel to graphics ones.
		Copy = VK_PIPELINE_STAGE_2_COPY_BIT,
		Blit = VK_PIPELINE_STAGE_2_BLIT_BIT,
		Clear = VK_PIPELINE_STAGE_2_CLEAR_BIT
	};

	enum class Access : VkAccessFlags2
	{
		None = VK_ACCESS_2_NONE,
		Read = VK_ACCESS_2_MEMORY_READ_BIT,
		TransferRead = VK_ACCESS_2_TRANSFER_READ_BIT,
		TransferWrite = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		UniformRead = VK_ACCESS_2_UNIFORM_READ_BIT,
		ShaderSampledRead = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
		ShaderStorageRead = VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
		ColorRead = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
		ColorWrite = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		DepthStencilRead = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		DepthStencilWrite = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};

	class VulkanEnum : private StaticClass
	{
	private:
		static ImageUsage FillUsageFlags(uint32_t flags);

	public:
		static void Startup(VkPhysicalDevice device);
		static ImageFormat FindSuitableImageFormat(ImageFormat format, ImageUsage usages);
		static ImageUsage GetFormatCapabilities(ImageFormat format);
		static VkFormat GetImageFormat(ImageFormat format);
		static bool IsColorFormat(ImageFormat format) { return format < ImageFormat::Depth16; }
		static bool IsStencilFormat(ImageFormat format) { return format > ImageFormat::Depth32; }
		static VkImageUsageFlags GetImageUsage(ImageUsage usages) { return static_cast<VkImageUsageFlags>(usages); }
		static VkImageLayout GetImageLayout(ImageLayout layout) { return layout == ImageLayout::ReadyToPresent ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : static_cast<VkImageLayout>(layout); }
		static VkImageAspectFlags GetImageAspect(ImageAspect aspect) { return static_cast<VkImageAspectFlags>(aspect); }
		static VkImageViewType GetImageType(ImageType type) { return static_cast<VkImageViewType>(type); }
		static VkFormat GetDataFormat(DataType type);
		static char const* GetDataTypeName(DataType type);
		static gl::DataType GetFormatForFloat(uint32_t componentCount) { return static_cast<gl::DataType>(componentCount - 1); }
		static gl::DataType GetFormatForInt(bool isSigned, uint32_t componentCount) { return static_cast<gl::DataType>(componentCount + isSigned ? 3 : 7); }
		static VkCullModeFlags GetCullMode(CullMode cullMode) { return static_cast<VkCullModeFlags>(cullMode); }
		static VkBlendFactor GetBlendFactor(BlendFactor blendFactor) { return static_cast<VkBlendFactor>(blendFactor); }
		static VkBlendOp GetBlendOperation(BlendOperation blendOperation) { return static_cast<VkBlendOp>(blendOperation); }
		static VkDescriptorType GetDescriptorType(DescriptorType type) { return static_cast<VkDescriptorType>(type); }
		static VkShaderStageFlags GetShaderStage(ShaderStage stages) { return static_cast<VkShaderStageFlags>(stages); }
		static VkBufferUsageFlags GetBufferUsage(BufferUsage usages) { return static_cast<VkBufferUsageFlags>(usages); }
		static VkAttachmentLoadOp GetAttachmentLoadOperation(AttachmentLoadOperation operation) { return static_cast<VkAttachmentLoadOp>(operation); }
		static VkAttachmentStoreOp GetAttachmentStoreOperation(AttachmentStoreOperation operation) { return static_cast<VkAttachmentStoreOp>(operation); }
	};
}