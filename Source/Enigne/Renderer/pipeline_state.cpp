#include "Engine/Renderer/pipeline_state.h"
#include "Engine/Renderer/renderer.h"
#include "Core/log.h"

using namespace glex;

/*————————————————————————————————————————————————————————————————————————————————————————————————————
		Pipeline state.
————————————————————————————————————————————————————————————————————————————————————————————————————*/
PipelineState::PipelineState(SharedPtr<Shader> const& shader, gl::MetaMaterialInfo metaMaterial, gl::RenderPass renderPass, uint32_t subpass) : m_shader(shader), m_metaMaterial(metaMaterial)
{
	gl::PipelineInfo pipeInfo;
	pipeInfo.vertexLayout = shader->GetVertexLayout();
	pipeInfo.metaMaterial = metaMaterial;
	pipeInfo.renderPass = renderPass;
	pipeInfo.subpass = subpass;
	pipeInfo.descriptorLayout = shader->GetDescriptorLayout();
	pipeInfo.vertexShader = shader->GetVertexShader();
	pipeInfo.geometryShader = shader->GetGeometryShader();
	pipeInfo.fragmentShader = shader->GetFragmentShader();
	m_pipelineObject.Create(pipeInfo);
}

PipelineState::~PipelineState()
{
	if (m_pipelineObject.GetHandle() != VK_NULL_HANDLE)
	{
		Renderer::PendingDelete([pipe = m_pipelineObject]() mutable
		{
			pipe.Destroy();
		});
	}
}

//bool PipelineState::Recreate(gl::RenderPass renderPass, uint32_t subpass)
//{
//	GLEX_DEBUG_ASSERT(IsValid()) {}
//	Renderer::PendingDelete([pipe = m_pipelineObject]() mutable
//	{
//		pipe.Destroy();
//	});
//	gl::PipelineInfo pipeInfo;
//	pipeInfo.vertexLayout = m_shader->GetVertexLayout();
//	pipeInfo.metaMaterial = m_metaMaterial;
//	pipeInfo.renderPass = renderPass;
//	pipeInfo.subpass = subpass;
//	pipeInfo.descriptorLayout = m_shader->GetDescriptorLayout();
//	pipeInfo.vertexShader = m_shader->GetVertexShader();
//	pipeInfo.geometryShader = m_shader->GetGeometryShader();
//	pipeInfo.fragmentShader = m_shader->GetFragmentShader();
//	if (m_pipelineObject.Create(pipeInfo))
//		return true;
//	else
//	{
//		m_pipelineObject = gl::PipelineState();
//		return false;
//	}
//}