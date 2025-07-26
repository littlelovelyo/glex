#include "Engine/Renderer/matinst.h"
#include "Engine/Renderer/renderer.h"

using namespace glex;

MaterialInstance::MaterialInstance(SharedPtr<Material> const& material, uint32_t materialDomain, SharedPtr<Shader> const& rebindShader)
{
	// Validity check.
	if (!material->IsValid())
		return;
	if (rebindShader != nullptr)
	{
		if (!rebindShader->IsValid())
			return;
		m_shader = rebindShader;
	}
	m_shader = material->GetShader();
	m_material = material;

	auto [renderPass, subpass, metaMaterial] = Renderer::GetRenderPipeline()->ResolveMaterialDomain(materialDomain);
	m_pipelineObject = Renderer::GetPipelineStateCache().GetPipelineState(m_shader, metaMaterial, renderPass.GetRenderPassObject(), subpass);
	m_metaMaterial = metaMaterial;
	if (m_pipelineObject.GetHandle() == VK_NULL_HANDLE)
	{
		m_material = nullptr;
		m_shader = nullptr;
	}
}

MaterialInstance::~MaterialInstance()
{
	if (m_pipelineObject.GetHandle() != VK_NULL_HANDLE)
		Renderer::GetPipelineStateCache().FreePipelineState(m_pipelineObject);
}

void MaterialInstance::Bind()
{
	WeakPtr<MaterialInstance>& currentMaterialInstance = Renderer::GetCurrentMaterialInstance();
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	if (currentMaterialInstance != this)
	{
		if (currentMaterialInstance == nullptr || currentMaterialInstance->m_pipelineObject != m_pipelineObject)
		{
			commandBuffer.BindPipelineState(m_pipelineObject);
			if (currentMaterialInstance == nullptr)
			{
				gl::DescriptorSet globalSet = Renderer::GetRenderPipeline()->GetGlobalDescriptorSet();
				if (globalSet.GetHandle() != VK_NULL_HANDLE)
					commandBuffer.BindDescriptorSet(m_shader->GetDescriptorLayout(), Renderer::GLOBAL_DESCRIPTOR_SET, Renderer::GetRenderPipeline()->GetGlobalDescriptorSet());
			}
		}
		if (m_material->GetDescriptorSet().GetHandle() != VK_NULL_HANDLE)
			commandBuffer.BindDescriptorSet(m_shader->GetDescriptorLayout(), Renderer::MATERIAL_DESCRIPOR_SET, m_material->GetDescriptorSet());
		currentMaterialInstance = this;
	}
}