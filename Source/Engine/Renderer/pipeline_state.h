/**
 * The structure is:
 *               Shader
 *               /    \
 *   PipelineState    Material
 *               \    /
 *          MaterialInstance (maybe)
 * 
 * We make pipeline state and material separate so we don't create a bunch of same materials.
 * Make them recreatable later.
 */
#pragma once
#include "Core/Memory/shared_ptr.h"
#include "Engine/Renderer/shader.h"

namespace glex
{
	class RenderPass;

	class PipelineState : private Unmoveable
	{
	private:
		gl::PipelineState m_pipelineObject;
		SharedPtr<Shader> m_shader;
		gl::MetaMaterialInfo m_metaMaterial;

	public:
		PipelineState(SharedPtr<Shader> const& shader, gl::MetaMaterialInfo metaMaterial, gl::RenderPass renderPass, uint32_t subpass);
		~PipelineState();		
		bool IsValid() const { return m_pipelineObject.GetHandle() != VK_NULL_HANDLE; }
		gl::PipelineState GetPipelineObject() const { return m_pipelineObject; }
		// bool Recreate(gl::RenderPass renderPass, uint32_t subpass);
	};
}