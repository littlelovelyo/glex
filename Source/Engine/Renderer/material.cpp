#include "Engine/Renderer/material.h"
#include "Engine/Renderer/renderer.h"
#include "Core/log.h"

using namespace glex;

MaterialInitializer::MaterialInitializer(SharedPtr<Shader> const& shader)
{
	if (shader != nullptr)
	{
		m_shader = shader;
		m_uniformBufferData = Mem::Alloc(shader->UniformBufferSize(), 4);
		memset(m_uniformBufferData, 0, shader->UniformBufferSize());
		m_textures.resize(shader->NumTextureArrays(), { UINT_MAX, {} });
	}
}

MaterialInitializer::MaterialInitializer(SharedPtr<Shader> const& shader, void* data, uint32_t size)
{
	if (shader != nullptr)
	{
		if (size != shader->UniformBufferSize())
			return;
		m_shader = shader;
		m_uniformBufferData = data;
	}
}

MaterialInitializer::~MaterialInitializer()
{
	Mem::Free(m_uniformBufferData);
	for (gl::PipelineState state : m_pipelineStates)
	{
		if (state.GetHandle() != VK_NULL_HANDLE)
			Renderer::GetPipelineStateCache().FreePipelineState(state);
	}
}

bool MaterialInitializer::SetFloat(char const* name, float value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::Float)
		{
			Logger::Error("property %s is not a float.", name);
			return false;
		}
		*Mem::Offset<float>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetVec2(char const* name, glm::vec2 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::Vec2)
		{
			Logger::Error("property %s is not a vec2.", name);
			return false;
		}
		*Mem::Offset<glm::vec2>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetVec3(char const* name, glm::vec3 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::Vec3)
		{
			Logger::Error("property %s is not a vec3.", name);
			return false;
		}
		*Mem::Offset<glm::vec3>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetVec4(char const* name, glm::vec4 const& value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::Vec4)
		{
			Logger::Error("property %s is not a vec4.", name);
			return false;
		}
		*Mem::Offset<glm::vec4>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetInt(char const* name, int32_t value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::Int)
		{
			Logger::Error("property %s is not a int.", name);
			return false;
		}
		*Mem::Offset<int32_t>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetIVec2(char const* name, glm::ivec2 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::IVec2)
		{
			Logger::Error("property %s is not a ivec2.", name);
			return false;
		}
		*Mem::Offset<glm::ivec2>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetIVec3(char const* name, glm::ivec3 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::IVec3)
		{
			Logger::Error("property %s is not a ivec3.", name);
			return false;
		}
		*Mem::Offset<glm::ivec3>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetIVec4(char const* name, glm::ivec4 const& value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::IVec4)
		{
			Logger::Error("property %s is not a ivec4.", name);
			return false;
		}
		*Mem::Offset<glm::ivec4>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetUInt(char const* name, uint32_t value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::UInt)
		{
			Logger::Error("property %s is not a uint.", name);
			return false;
		}
		*Mem::Offset<uint32_t>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetUVec2(char const* name, glm::uvec2 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::UVec2)
		{
			Logger::Error("property %s is not a uvec2.", name);
			return false;
		}
		*Mem::Offset<glm::uvec2>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetUVec3(char const* name, glm::uvec3 value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::UVec3)
		{
			Logger::Error("property %s is not a uvec3.", name);
			return false;
		}
		*Mem::Offset<glm::uvec3>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetUVec4(char const* name, glm::uvec4 const& value)
{
	if (m_shader != nullptr)
	{
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Vector || prop.vector.type != gl::DataType::UVec4)
		{
			Logger::Error("property %s is not a uvec4.", name);
			return false;
		}
		*Mem::Offset<glm::uvec4>(m_uniformBufferData, prop.vector.offset) = value;
		return true;
	}
	return false;
}

bool MaterialInitializer::SetTexture(char const* name, uint32_t index, SharedPtr<Texture> const& texture)
{
	if (m_shader != nullptr)
	{
		if (!texture->IsValid())
			return false;
		ShaderProperty prop = m_shader->GetProperty(name);
		if (prop.type != ShaderPropertyType::Texture)
		{
			Logger::Error("Property %s is not a texture.", name);
			return false;
		}
		if (prop.texture.type != texture->GetImageView().Type())
		{
			Logger::Error("Type of property %s does not match input parameter.", name);
			return false;
		}
		for (auto& entry : m_textures)
		{
			if (entry.first == prop.texture.index || entry.first == UINT_MAX)
			{
				if (entry.first == UINT_MAX)
					entry.first = prop.texture.index;
				auto& list = entry.second;
				list.resize(glm::max(list.size(), index + 1));
				list[index] = texture;
			}
		}
		return true;
	}
	return false;
}

bool MaterialInitializer::AddMaterialDomain(uint32_t materialDomain, SharedPtr<Shader> shader)
{
	m_pipelineStates.resize(glm::max(m_pipelineStates.size(), materialDomain + 1));
	auto [renderPass, subpass, metaMaterial] = Renderer::GetRenderPipeline()->ResolveMaterialDomain(materialDomain);
	gl::PipelineState state = Renderer::GetPipelineStateCache().GetPipelineState(shader, metaMaterial, renderPass.GetRenderPassObject(), subpass);
	if (state.GetHandle() == VK_NULL_HANDLE)
		return false;
	gl::PipelineState& old = m_pipelineStates[materialDomain];
	if (old.GetHandle() != VK_NULL_HANDLE)
		Renderer::GetPipelineStateCache().FreePipelineState(old);
	old = state;
	return true;
}

Material::Material(MaterialInitializer& init) : m_shader(std::move(init.m_shader))
{
	if (m_shader == nullptr)
		return;

	if (m_shader->GetMaterialLayout().GetHandle() != VK_NULL_HANDLE)
	{
		Vector<gl::Descriptor> descriptors;
		gl::BufferDescriptor uniformBuffer;
		Vector<gl::ImageSamplerDesciptor> imageSamplers;

		if (m_shader->UniformBufferSize() != 0)
		{
			m_uniformBuffer.Emplace(gl::BufferUsage::Uniform | gl::BufferUsage::TransferDest, m_shader->UniformBufferSize(), false);
			if (!m_uniformBuffer->IsValid())
			{
				m_shader = nullptr;
				return;
			}

			Renderer::UploadBuffer(&m_uniformBuffer, 0, m_shader->UniformBufferSize(), init.m_uniformBufferData);
			uniformBuffer.buffer = m_uniformBuffer->GetBufferObject();
			uniformBuffer.offset = 0;
			uniformBuffer.size = m_shader->UniformBufferSize();

			gl::Descriptor uniformData;
			uniformData.type = gl::DescriptorType::UniformBuffer;
			uniformData.bindingPoint = 0;
			uniformData.arrayIndex = 0;
			uniformData.buffers = &uniformBuffer;
			descriptors.push_back(uniformData);
		}
		for (auto& [location, list] : init.m_textures)
		{
			imageSamplers.resize(imageSamplers.size() + list.size());
			for (uint32_t i = 0; i < list.size(); i++)
			{
				gl::ImageSamplerDesciptor& descriptor = imageSamplers[imageSamplers.size() - list.size() + i];
				SharedPtr<Texture> const& texture = list[i];
				if (texture != nullptr)
				{
					descriptor.image.imageView = texture->GetImageView().GetImageViewObject();
					descriptor.image.imageLayout = gl::ImageLayout::ShaderRead;
					descriptor.sampler.sampler = texture->GetSampler();
				}
				else
					Logger::Warn("Texture at location %d, index %d is not set.", location, i);
			}
			gl::Descriptor texture;
			texture.type = gl::DescriptorType::CombinedImageSampler;
			texture.bindingPoint = location;
			texture.arrayIndex = 0;
			texture.imageSamplers = std::pair { imageSamplers.end() - list.size(), list.size() };
			descriptors.push_back(texture);
		}
		m_descriptorSet = Renderer::AllocateStaticMaterialDescriptorSet(m_shader->GetMaterialLayout());
		m_descriptorSet.BindDescriptors(descriptors);
	}
	m_pipelineStates = std::move(init.m_pipelineStates);
}

Material::~Material()
{
	if (m_shader != nullptr)
	{
		if (m_shader->GetMaterialLayout().GetHandle() != VK_NULL_HANDLE)
		{
			Renderer::FreeStaticMaterialDescriptorSet(m_descriptorSet);
			if (m_shader->UniformBufferSize())
				m_uniformBuffer.Destroy();
		}
		for (gl::PipelineState state : m_pipelineStates)
		{
			if (state.GetHandle() != VK_NULL_HANDLE)
				Renderer::GetPipelineStateCache().FreePipelineState(state);
		}
	}
}