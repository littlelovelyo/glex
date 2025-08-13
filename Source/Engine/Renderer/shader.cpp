#include "Engine/Renderer/shader.h"
#include "Engine/Renderer/renderer.h"
#include "Core/Platform/filesync.h"
#include "Core/Utils/string.h"
#include "Core/Utils/raii.h"
#include "Core/log.h"
#include <SPIRV-Reflect/spirv_reflect.h>
#include <numeric>
#include <algorithm>

using namespace glex;

bool Shader::FillDescriptorLayout(char const* shaderFile, gl::ShaderStage stage, SpvReflectShaderModule* reflectModule, std::array<Vector<gl::DescriptorBinding>, Limits::NUM_DESCRIPTOR_SETS>& descriptorLayout, gl::ShaderStage& constantStages)
{
	uint32_t numDescriptorSets;
	if (spvReflectEnumerateDescriptorSets(reflectModule, &numDescriptorSets, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
	{
		Logger::Error("Cannot reflect shader: %s.", shaderFile);
		return false;
	}
	Vector<SpvReflectDescriptorSet*> descriptorSets(numDescriptorSets);
	if (spvReflectEnumerateDescriptorSets(reflectModule, &numDescriptorSets, descriptorSets.data()))
	{
		Logger::Error("Cannot reflect shader: %s.", shaderFile);
		return false;
	}

	for (SpvReflectDescriptorSet* set : descriptorSets)
	{
		if (set->set >= Limits::NUM_DESCRIPTOR_SETS)
		{
			Logger::Error("Number of descriptor sets exceeds the maximun count in %s.", shaderFile);
			return false;
		}
		if (set->binding_count > Limits::NUM_BINDINGS_PER_SET)
		{
			Logger::Error("Too many bindings in shader: %s.", shaderFile);
			return false;
		}
		Vector<gl::DescriptorBinding>& bindings = descriptorLayout[set->set];
		for (uint32_t i = 0; i < set->binding_count; i++)
		{
			SpvReflectDescriptorBinding* descriptor = set->bindings[i];
			gl::DescriptorType descType = static_cast<gl::DescriptorType>(descriptor->descriptor_type);
			uint32_t count = std::accumulate(descriptor->array.dims, descriptor->array.dims + descriptor->array.dims_count, 1, [](uint32_t lhs, uint32_t rhs) { return lhs * rhs; });
			gl::DescriptorBinding* binding = eastl::find(bindings.begin(), bindings.end(), descriptor->binding, [](gl::DescriptorBinding const& lhs, uint32_t rhs) { return lhs.bindingPoint == rhs; });
			if (binding != bindings.end())
			{
				if (descType != binding->type || count != binding->arraySize || (binding->shaderStage & stage) != gl::ShaderStage::None) // May be redundant.
				{
					Logger::Error("Descriptor doesn't match its previous definition: %s.", shaderFile);
					return false;
				}
				binding->shaderStage = binding->shaderStage | stage;
			}
			else
			{
				binding = &bindings.emplace_back();
				binding->bindingPoint = descriptor->binding;
				binding->arraySize = count;
				binding->type = descType;
				binding->shaderStage = stage;
			}

			// Material property reflection.
			if (set->set == Renderer::MATERIAL_DESCRIPOR_SET)
			{
				if (descType == gl::DescriptorType::UniformBuffer)
				{
					if (descriptor->binding != 0)
					{
						Logger::Error("Uniform buffer of a material must be bound to index 0. Error occured in shader: %s.", shaderFile);
						return false;
					}
					if (count != 1)
					{
						Logger::Error("Uniform buffer of a material must not be an array. Error occured in shader: %s.", shaderFile);
						return false;
					}
					SpvReflectBlockVariable& def = descriptor->block;
					if (def.size > Limits::UNIFORM_BUFFER_SIZE)
					{
						Logger::Error("Uniform buffer is too large in shader: %s.", shaderFile);
						return false;
					}
					m_uniformBufferSize = glm::max<uint16_t>(m_uniformBufferSize, def.size);
					for (uint32_t j = 0; j < def.member_count; j++)
					{
						SpvReflectBlockVariable& member = def.members[j];
						// Why do we ever need to support matrix anyway?
						constexpr uint32_t ALLOWED_TYPE_FLAGS = SPV_REFLECT_TYPE_FLAG_INT | SPV_REFLECT_TYPE_FLAG_FLOAT | SPV_REFLECT_TYPE_FLAG_VECTOR;
						if ((member.type_description->type_flags & ~ALLOWED_TYPE_FLAGS) || member.numeric.scalar.width != 32)
						{
							Logger::Error("Only 32-bit scalar and vector types are allowed in material definition. Error occured in shader: %s.", shaderFile);
							return false;
						}
						// Now deduce its type. There may be other overlapping issues, but we can't really check everything.
						char const* name = member.name;
						if (member.numeric.vector.component_count == 0)
							member.numeric.vector.component_count = 1; // Fix.
						gl::DataType dataType = member.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT ? gl::VulkanEnum::GetFormatForFloat(member.numeric.vector.component_count) : gl::VulkanEnum::GetFormatForInt(member.numeric.scalar.signedness, member.numeric.vector.component_count);
						uint32_t offset = member.offset;
						auto iter = m_properties.find_as(name);
						if (iter == m_properties.end())
						{
							ShaderProperty& property = m_properties[name];
							property.type = ShaderPropertyType::Vector;
							property.vector.type = dataType;
							property.vector.offset = offset;
						}
						else
						{
							ShaderProperty& property = iter->second;
							if (property.type != ShaderPropertyType::Vector || property.vector.type != dataType || property.vector.offset != offset)
							{
								Logger::Error("Material definition in shader %s doesn't match its previous definition.", shaderFile);
								return false;
							}
						}
					}
				}
				else if (descType == gl::DescriptorType::CombinedImageSampler)
				{
					auto iter = m_properties.find_as(descriptor->name);
					if (iter == m_properties.end())
					{
						ShaderProperty& property = m_properties[descriptor->name];
						property.type = ShaderPropertyType::Texture;
						property.texture.type = static_cast<gl::ImageType>(descriptor->image.dim);
						property.texture.index = descriptor->binding;
						property.texture.arraySize = count;
					}
					else
					{
						ShaderProperty& property = iter->second;
						if (property.type != ShaderPropertyType::Texture || *property.texture.type != descriptor->image.dim || property.texture.index != descriptor->binding || property.texture.arraySize != count)
						{
							Logger::Error("Material definition in shader %s doesn't match its previous definition.", shaderFile);
							return false;
						}
					}
				}
				else
				{
					Logger::Error("Descriptor type %d in shader %s cannot be used as a material property.", *descType, shaderFile);
					return false;
				}
			}
		}
	}

	uint32_t numPushConstants;
	if (spvReflectEnumeratePushConstantBlocks(reflectModule, &numPushConstants, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
	{
		Logger::Error("Cannot reflect shader: %s.", shaderFile);
		return false;
	}
	if (numPushConstants > 1)
	{
		Logger::Error("More than 1 push-constants buffer is not supported. Error occured in shader: %s.", shaderFile);
		return false;
	}
	if (numPushConstants == 1)
		constantStages = constantStages | stage;
	return true;
}

Shader::Shader(ShaderInitializer const& init)
{
	GLEX_DEBUG_ASSERT(init.vertexShaderFile != nullptr && init.fragmentShaderFile != nullptr) {}
	render::DescriptorLayoutType descriptorLayout;

	
	///////////////////////////////////////
	//////////// Vertex shader ////////////
	{
		auto [vertexSource, vertexShaderSize] = FileSync::ReadAllContent(init.vertexShaderFile);
		if (vertexSource == nullptr)
			return;

		SpvReflectShaderModule reflectModule;
		if (spvReflectCreateShaderModule(vertexShaderSize, vertexSource, &reflectModule) != SPV_REFLECT_RESULT_SUCCESS)
		{
			Logger::Error("Cannot reflect shader: %s.", init.vertexShaderFile);
			return;
		}
		AutoCleaner vrefCleaner([&]() { spvReflectDestroyShaderModule(&reflectModule); });

		// Vertex layout.
		uint32_t numVertexAttributes;
		if (spvReflectEnumerateInputVariables(&reflectModule, &numVertexAttributes, nullptr) != SPV_REFLECT_RESULT_SUCCESS)
		{
			Logger::Error("Cannot reflect shader: %s.", init.vertexShaderFile);
			return;
		}
		if (numVertexAttributes > Limits::NUM_VERTEX_ATTRIBUTES)
		{
			Logger::Error("Too many vertex attributes in %s.", init.vertexShaderFile);
			return;
		}
		m_numVertexAttributes = numVertexAttributes;
		Vector<SpvReflectInterfaceVariable*> inputVariables(m_numVertexAttributes);
		if (spvReflectEnumerateInputVariables(&reflectModule, &numVertexAttributes, inputVariables.data()) != SPV_REFLECT_RESULT_SUCCESS)
		{
			Logger::Error("Cannot reflect shader: %s.", init.vertexShaderFile);
			return;
		}
		for (uint32_t i = 0; i < m_numVertexAttributes; i++)
		{
			SpvReflectInterfaceVariable* inputVariable = inputVariables[i];
			if (i >= m_numVertexAttributes)
			// if (inputVariable->location != i)
			{
				Logger::Error("Incontinuous vertex attribute location is not supported. Error occured in vertex shader: %s.", init.vertexShaderFile);
				return;
			}
			switch (inputVariable->format)
			{
				case SPV_REFLECT_FORMAT_R32_SFLOAT: m_vertexLayout[i] = gl::DataType::Float; break;
				case SPV_REFLECT_FORMAT_R32G32_SFLOAT:  m_vertexLayout[i] = gl::DataType::Vec2; break;
				case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:  m_vertexLayout[i] = gl::DataType::Vec3; break;
				case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:  m_vertexLayout[i] = gl::DataType::Vec4; break;
				case SPV_REFLECT_FORMAT_R32_SINT:  m_vertexLayout[i] = gl::DataType::Int; break;
				case SPV_REFLECT_FORMAT_R32G32_SINT:  m_vertexLayout[i] = gl::DataType::IVec2; break;
				case SPV_REFLECT_FORMAT_R32G32B32_SINT:  m_vertexLayout[i] = gl::DataType::IVec3; break;
				case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:  m_vertexLayout[i] = gl::DataType::IVec4; break;
				case SPV_REFLECT_FORMAT_R32_UINT:  m_vertexLayout[i] = gl::DataType::UInt; break;
				case SPV_REFLECT_FORMAT_R32G32_UINT:  m_vertexLayout[i] = gl::DataType::UVec2; break;
				case SPV_REFLECT_FORMAT_R32G32B32_UINT:  m_vertexLayout[i] = gl::DataType::UVec3; break;
				case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:  m_vertexLayout[i] = gl::DataType::UVec4; break;
				default: Logger::Error("Unsupported vertex attribute type in vertex shader: %s.", init.vertexShaderFile); return;
			}
		}

		if (!FillDescriptorLayout(init.vertexShaderFile, gl::ShaderStage::Vertex, &reflectModule, descriptorLayout, m_pushConstantsStages))
			return;
		m_vertexShader = Renderer::GetShaderModuleCache().GetShaderModule(init.vertexShaderFile, vertexSource, vertexShaderSize);
		if (m_vertexShader.GetHandle() == VK_NULL_HANDLE)
			return;
	}
	AutoCleaner vertexShaderCleaner([this]()
	{
		if (m_descriptorLayout.GetHandle() == VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_vertexShader);
	});

	/**
	 * Geometry shader.
	 */
	if (init.geometryShaderFile != nullptr)
	{
		auto [geometrySource, gemoetryShaderSize] = FileSync::ReadAllContent(init.geometryShaderFile);
		if (geometrySource == nullptr)
			return;

		SpvReflectShaderModule reflectModule;
		if (spvReflectCreateShaderModule(gemoetryShaderSize, geometrySource, &reflectModule) != SPV_REFLECT_RESULT_SUCCESS)
		{
			Logger::Error("Cannot reflect shader: %s.", init.vertexShaderFile);
			return;
		}
		AutoCleaner vrefCleaner([&]() { spvReflectDestroyShaderModule(&reflectModule); });

		if (!FillDescriptorLayout(init.geometryShaderFile, gl::ShaderStage::Geometry, &reflectModule, descriptorLayout, m_pushConstantsStages))
			return;
		m_geometryShader = Renderer::GetShaderModuleCache().GetShaderModule(init.geometryShaderFile, geometrySource, gemoetryShaderSize);
		if (m_geometryShader.GetHandle() == VK_NULL_HANDLE)
			return;
	}
	AutoCleaner geometryShaderCleaner([this]()
	{
		if (m_geometryShader.GetHandle() != VK_NULL_HANDLE && m_descriptorLayout.GetHandle() == VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_geometryShader);
	});

	/**
	 * Fragment shader.
	 */
	{
		auto [fragmentSource, fragmentShaderSize] = FileSync::ReadAllContent(init.fragmentShaderFile);
		if (fragmentSource == nullptr)
			return;

		SpvReflectShaderModule reflectModule;
		if (spvReflectCreateShaderModule(fragmentShaderSize, fragmentSource, &reflectModule) != SPV_REFLECT_RESULT_SUCCESS)
		{
			Logger::Error("Cannot reflect shader: %s.", init.vertexShaderFile);
			return;
		}
		AutoCleaner vrefCleaner([&]() { spvReflectDestroyShaderModule(&reflectModule); });

		if (!FillDescriptorLayout(init.fragmentShaderFile, gl::ShaderStage::Fragment, &reflectModule, descriptorLayout, m_pushConstantsStages))
			return;
		m_fragmentShader = Renderer::GetShaderModuleCache().GetShaderModule(init.fragmentShaderFile, fragmentSource, fragmentShaderSize);
		if (m_fragmentShader.GetHandle() == VK_NULL_HANDLE)
			return;
	}
	AutoCleaner fragmentShaderCleaner([this]()
	{
		if (m_descriptorLayout.GetHandle() == VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_fragmentShader);
	});

	{
		HashSet<uint32_t> textureBindingPoints;
		for (gl::DescriptorBinding const& descriptor : descriptorLayout[Renderer::MATERIAL_DESCRIPOR_SET])
		{
			if (descriptor.type == gl::DescriptorType::CombinedImageSampler)
			{
				textureBindingPoints.insert(descriptor.bindingPoint);
				m_numTextures += descriptor.arraySize;
			}
		}
		if (m_numTextures > Limits::NUM_MATERIAL_TEXTURES)
		{
			Logger::Error("Too many textures.");
			return;
		}
		m_numTextureArrays = textureBindingPoints.size();

		for (auto& list : descriptorLayout)
		{
			std::sort(list.begin(), list.end(), [](gl::DescriptorBinding const& lhs, gl::DescriptorBinding const& rhs)
			{
				return lhs.bindingPoint < rhs.bindingPoint;
			});
		}
		m_descriptorLayout = Renderer::GetDescriptorLayoutCache().GetDescriptorLayout(descriptorLayout, m_pushConstantsStages, m_materialLayout, m_objectLayout);
		if (m_descriptorLayout.GetHandle() == VK_NULL_HANDLE)
		{
			Logger::Error("Cannot create descriptor layout.");
			return;
		}
#if GLEX_COMMON_LOGGING
		Log(init.vertexShaderFile, init.geometryShaderFile, init.fragmentShaderFile);
#endif
	}
}

Shader::~Shader()
{
	if (m_descriptorLayout.GetHandle() != VK_NULL_HANDLE)
	{
		Renderer::GetDescriptorLayoutCache().FreeDescriptorLayout(m_descriptorLayout);
		if (m_vertexShader.GetHandle() != VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_vertexShader);
		if (m_geometryShader.GetHandle() != VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_geometryShader);
		if (m_fragmentShader.GetHandle() != VK_NULL_HANDLE)
			Renderer::GetShaderModuleCache().FreeShaderModule(m_fragmentShader);
	}
}

void Shader::Log(char const* vertexShaderFile, char const* geometryShaderFile, char const* fragmentShaderFile)
{
	char buffer[Limits::LOG_BUFFER_SIZE];
	memcpy(buffer, R"(+----------------------------------+
|
|    SHADER INFORMATION
|
| Vertex shader: )", 82);
	uint32_t ptr = 82;

	char const* strippedFile = StringUtils::GetFileName(vertexShaderFile);
	uint32_t length = strlen(strippedFile);
	if (ptr + length >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, strippedFile, length);
	ptr += length;

	if (geometryShaderFile != nullptr)
	{
		if (ptr + 20 >= Limits::LOG_BUFFER_SIZE)
			goto END;
		memcpy(buffer, R"(
| Geometry shader: )", 20);
		ptr += 20;

		strippedFile = StringUtils::GetFileName(geometryShaderFile);
		length = strlen(strippedFile);
		if (ptr + length >= Limits::LOG_BUFFER_SIZE)
			goto END;
		memcpy(buffer + ptr, strippedFile, length);
		ptr += length;
	}

	if (ptr + 20 >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, R"(
| Fragment shader: )", 20);
	ptr += 20;

	strippedFile = StringUtils::GetFileName(fragmentShaderFile);
	length = strlen(strippedFile);
	if (ptr + length >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, strippedFile, length);
	ptr += length;

	/*if (ptr + 22 >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, R"(
| Descriptor layout: )", 22);
	ptr += 22;
	if (ptr + descriptorLayout.length() >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, descriptorLayout.data(), descriptorLayout.length());
	ptr += descriptorLayout.length();*/

	if (ptr + 69 >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, R"(
+----------------------------------+
|
|    MATERIAL DEFINITION
|
| )", 69);
	ptr += 69;

	for (auto& [name, prop] : m_properties)
	{
		if (ptr + name.length() >= Limits::LOG_BUFFER_SIZE)
			goto END;
		memcpy(buffer + ptr, name.c_str(), name.length());
		ptr += name.length();

		if (ptr + 2 >= Limits::LOG_BUFFER_SIZE)
			goto END;
		memcpy(buffer + ptr, ": ", 2);
		ptr += 2;

		if (prop.type == ShaderPropertyType::Vector)
		{
			char const* typeName = gl::VulkanEnum::GetDataTypeName(prop.vector.type);
			length = strlen(typeName);
			if (ptr + length >= Limits::LOG_BUFFER_SIZE)
				goto END;
			memcpy(buffer + ptr, typeName, length);
			ptr += length;
		}
		else
		{
			length = StringUtils::ToString(prop.texture.index, buffer + ptr, Limits::LOG_BUFFER_SIZE - ptr - 1);
			if (length == 0)
				goto END;
			ptr += length;
		}

		if (ptr + 3 >= Limits::LOG_BUFFER_SIZE)
			goto END;
		memcpy(buffer + ptr, R"(
| )", 3);
		ptr += 3;
	}

	ptr -= 2;
	if (ptr + 36 >= Limits::LOG_BUFFER_SIZE)
		goto END;
	memcpy(buffer + ptr, R"(+----------------------------------+)", 36);
	ptr += 36;

END:
	buffer[ptr] = 0;
	Logger::Info(buffer);
}

ShaderProperty Shader::GetProperty(char const* name) const
{
	auto iter = m_properties.find_as(name);
	if (iter != m_properties.end())
		return iter->second;
	ShaderProperty result;
	result.type = ShaderPropertyType::Invalid;
	return result;
}