#pragma once
#include "Core/GL/shader.h"
#include "Core/GL/pipeline_state.h"
#include "Core/Utils/temp_buffer.h"
#include "Engine/resbase.h"
#include <array>

struct SpvReflectShaderModule;

namespace glex
{
	namespace render
	{
		using DescriptorLayoutType = std::array<Vector<gl::DescriptorBinding>, Limits::NUM_DESCRIPTOR_SETS>;
	}

	struct ShaderVectorProperty
	{
		gl::DataType type;
		uint16_t offset;
	};

	struct ShaderTextureProperty
	{
		gl::ImageType type;
		uint8_t index; // Can be 0 if uniform buffer is absent.
		uint8_t arraySize;
	};

	enum class ShaderPropertyType : uint8_t
	{
		Vector,
		Texture,
		Invalid
	};

	struct ShaderProperty
	{
		ShaderPropertyType type;
		union
		{
			ShaderVectorProperty vector;
			ShaderTextureProperty texture;
		};
	};

	struct ShaderInitializer
	{
		char const* vertexShaderFile;
		char const* geometryShaderFile;
		char const* fragmentShaderFile;
	};

	class Shader : public ResourceBase
	{
		friend class ResourceManager;

	private:
		gl::DescriptorSetLayout m_objectLayout;
		gl::DescriptorSetLayout m_materialLayout;
		gl::DescriptorLayout m_descriptorLayout;
		gl::Shader m_vertexShader, m_geometryShader, m_fragmentShader;
		uint16_t m_uniformBufferSize = 0;
		uint16_t m_numTextureArrays = 0;
		uint16_t m_numTextures = 0;
		gl::ShaderStage m_pushConstantsStages;
		uint8_t m_numVertexAttributes = 0;
		gl::DataType m_vertexLayout[Limits::NUM_VERTEX_ATTRIBUTES];
		HashMap<String, ShaderProperty> m_properties;

		Shader(ShaderInitializer const& init);
		bool IsValid() const { return m_descriptorLayout.GetHandle() != VK_NULL_HANDLE; }
		bool FillDescriptorLayout(char const* shaderFile, gl::ShaderStage stage, SpvReflectShaderModule* reflectModule, render::DescriptorLayoutType& descriptorLayout, gl::ShaderStage& constantStages);
		void Log(char const* vertexShaderFile, char const* geometryShaderFile, char const* fragmentShaderFile);

	public:
		~Shader();
		SequenceView<gl::DataType const> GetVertexLayout() const { return { m_vertexLayout, m_numVertexAttributes }; }
		gl::DescriptorLayout GetDescriptorLayout() const { return m_descriptorLayout; }
		gl::DescriptorSetLayout GetMaterialLayout() const { return m_materialLayout; }
		gl::DescriptorSetLayout GetObjectLayout() const { return m_objectLayout; }
		gl::ShaderStage GetPushConstantsStages() const { return m_pushConstantsStages; }
		gl::Shader GetVertexShader() const { return m_vertexShader; }
		gl::Shader GetGeometryShader() const { return m_geometryShader; }
		gl::Shader GetFragmentShader() const { return m_fragmentShader; }
		uint32_t UniformBufferSize() const { return m_uniformBufferSize; }
		uint32_t NumTextureArrays() const { return m_numTextureArrays; }
		uint32_t NumTextures() const { return m_numTextures; }
		ShaderProperty GetProperty(char const* name) const;
		HashMap<String, ShaderProperty> const& GetAllProperties() const { return m_properties; }
	};
}