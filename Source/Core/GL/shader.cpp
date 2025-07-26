#include "Core/GL/shader.h"
#include "Core/GL/context.h"

using namespace glex::gl;

bool Shader::Create(void const* bytecode, uint32_t size)
{
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = size;
	shaderInfo.pCode = reinterpret_cast<uint32_t const*>(bytecode);
	if (vkCreateShaderModule(Context::GetDevice(), &shaderInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void Shader::Destroy()
{
	vkDestroyShaderModule(Context::GetDevice(), m_handle, Context::HostAllocator());
}