#pragma once
#include "Engine/Renderer/shader.h"
#include "Engine/Renderer/texture.h"
#include "Engine/Renderer/buffer.h"
#include "Engine/resbase.h"
#include "Core/Memory/smart_ptr.h"

namespace glex
{
	class MaterialInitializer
	{
		friend class Material;

	private:
		SharedPtr<Shader> m_shader;
		void* m_uniformBufferData;
		Vector<std::pair<uint32_t, InlineVector<SharedPtr<Texture>, 2>>> m_textures;
		Vector<gl::PipelineState> m_pipelineStates;

	public:
		MaterialInitializer(SharedPtr<Shader> const& shader); // For template only.
		MaterialInitializer(SharedPtr<Shader> const& shader, void* data, uint32_t size);
		~MaterialInitializer();
		bool IsValid() const { return m_shader != nullptr; }
		bool SetFloat(char const* name, float value);
		bool SetVec2(char const* name, glm::vec2 value);
		bool SetVec3(char const* name, glm::vec3 value);
		bool SetVec4(char const* name, glm::vec4 const& value);
		bool SetInt(char const* name, int32_t value);
		bool SetIVec2(char const* name, glm::ivec2 value);
		bool SetIVec3(char const* name, glm::ivec3 value);
		bool SetIVec4(char const* name, glm::ivec4 const& value);
		bool SetUInt(char const* name, uint32_t value);
		bool SetUVec2(char const* name, glm::uvec2 value);
		bool SetUVec3(char const* name, glm::uvec3 value);
		bool SetUVec4(char const* name, glm::uvec4 const& value);
		bool SetTexture(char const* name, uint32_t index, SharedPtr<Texture> const& texture);
		bool AddMaterialDomain(uint32_t materialDomain, SharedPtr<Shader> shader);
	};

	/**
	 * Let material hold shader-pipeline pairs.
	 * Class relation can be simplified. Hooray!
	 */
	class Material : public ResourceBase
	{
	private:
		SharedPtr<Shader> m_shader;        // For template only.
		Optional<Buffer> m_uniformBuffer;
		gl::DescriptorSet m_descriptorSet; // Can be null if we don't have any parameters.
		Vector<gl::PipelineState> m_pipelineStates;

		Material(MaterialInitializer& init);
		bool IsValid() const { return m_shader != nullptr; }

	public:
		~Material();
		gl::DescriptorSet GetDescriptorSet() const { return m_descriptorSet; }
		gl::PipelineState GetPipelineState(uint32_t materialDomain) const { return m_pipelineStates[materialDomain]; }
	};
}