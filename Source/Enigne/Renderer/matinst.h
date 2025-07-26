/**
 * Material instance.
 * 
 * A combination of material and pipeline state.
 * Different from what's in Unreal Engine.
 * 
 */
#pragma once
#include "Engine/Renderer/material.h"

namespace glex
{
	class MaterialInstance : public ResourceBase
	{
	private:
		SharedPtr<Material> m_material;
		SharedPtr<Shader> m_shader;
		gl::PipelineState m_pipelineObject;
		gl::MetaMaterialInfo m_metaMaterial;

	public:
		MaterialInstance(SharedPtr<Material> const& material, uint32_t materialDomain, SharedPtr<Shader> const& rebindShader = nullptr);
		~MaterialInstance();
		bool IsValid() const { return m_pipelineObject.GetHandle() != VK_NULL_HANDLE; }
		void Bind();
		SharedPtr<Shader> const& GetShader() { return m_shader; }
		gl::PipelineState GetPipelineState() const { return m_pipelineObject; }
	};
}