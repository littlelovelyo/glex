/**
 * Vulkan shader.
 */
#pragma once
#include <vulkan/vulkan.h>

namespace glex::gl
{
	class Shader
	{
	private:
		VkShaderModule m_handle;

	public:
		constexpr static uint32_t k_bytecodeAlignment = 4;
		Shader() : m_handle(VK_NULL_HANDLE) {}
		Shader(VkShaderModule handle) : m_handle(handle) {}
		bool Create(void const* bytecode, uint32_t size);
		void Destroy();
		VkShaderModule GetHandle() const { return m_handle; }
	};
}