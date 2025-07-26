/**
 * Compilation configurations.
 */
#pragma once
#include <stdint.h>

#ifdef GLEX_DEBUG
#define GLEX_REPORT_GLFW_ERRORS 1	// It never happens.
#define GLEX_ENABLE_VALIDATION_LAYER 1
#else
#define GLEX_REPORT_GLFW_ERRORS 0
#define GLEX_ENABLE_VALIDATION_LAYER 0
#endif

#ifdef GLEX_RELEASE
#define GLEX_COMMON_LOGGING 0
#define GLEX_REPORT_GL_ERRORS 0
#define GLEX_REPORT_RESOURCE_ERRORS 0
#define GLEX_REPORT_MEMORY_LEAKS 0
#define GLEX_REPORT_SCRIPT_ERRORS 0
#define GLEX_REPORT_PHYSICS_ERRORS 0
#define GLEX_REPORT_PERFORMANCE_ISSUE 0
#define GLEX_DEBUG_RENDERING 0
#else
#define GLEX_COMMON_LOGGING 1
#define GLEX_REPORT_GL_ERRORS 1
#define GLEX_REPORT_RESOURCE_ERRORS 1
#define GLEX_REPORT_MEMORY_LEAKS 1
#define GLEX_REPORT_SCRIPT_ERRORS 1
#define GLEX_REPORT_PHYSICS_ERRORS 1
#define GLEX_REPORT_PERFORMANCE_ISSUE 1
#define GLEX_DEBUG_RENDERING 1
#endif

namespace glex
{
	class Limits
	{
	private:
		Limits() = delete;

	public:
		constexpr static uint32_t KB = 1024;
		constexpr static uint32_t MB = 1048576;
		constexpr static uint32_t GB = 1073741824;
		constexpr static uint32_t NAME_LENGTH = 63;
		constexpr static uint32_t PATH_LENGTH = 259;
		constexpr static uint32_t LOG_BUFFER_SIZE = 890;
		constexpr static uint32_t TEXTURE_SIZE = 8192;
		constexpr static uint32_t NUM_VERTEX_ATTRIBUTES = 16;              // Vulkan approved.
		constexpr static uint32_t UNIFORM_BUFFER_SIZE = 16 * KB;           // 16 KB, Vulkan approved.
		constexpr static uint32_t NUM_MATERIAL_TEXTURES = 16;              // Vulkan approved. We can have more.
		constexpr static uint32_t NUM_DESCRIPTOR_SETS = 4;
		constexpr static uint32_t NUM_BINDINGS_PER_SET = 18;
		constexpr static uint32_t DESCRIPTOR_LAYOUT_STRING_LENGTH = 519;   // Shouldn't be that long, right?
		constexpr static uint32_t RENDER_PASS_SEQUENCE_LENGTH = 520;

		constexpr static uint32_t NUM_BONES = 512;
		constexpr static uint32_t NUM_ANIMATIONS = 64;
		constexpr static uint32_t NUM_KEYFRAMES = 1024;
		constexpr static uint32_t VERTEX_BUFFER_SIZE = 64 * MB;
		constexpr static uint32_t INDEX_BUFFER_SIZE = 16 * MB;
		constexpr static uint32_t NUM_BATCH_TEXTURES = 64;
		constexpr static uint32_t NUM_RESOURCES = 1048576;
		constexpr static uint32_t NUM_OBJECTS = 1048576;
		constexpr static uint32_t NUM_COMPONENTS = 32;
	};
}

#define EASTL_SIZE_T_32BIT 1