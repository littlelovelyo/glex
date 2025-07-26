#pragma once
#include "Core/Container/sequence.h"
#include "Core/GL/shader.h"
#include "Core/GL/render_pass.h"
#include "Core/GL/descriptor.h"

namespace glex::gl
{
	struct MetaMaterialInfo
	{
		CullMode cullMode : 4 = CullMode::Back;
		bool depthTest : 1 = true;
		bool depthWrite : 1 = true;
		bool blend : 1 = false;
		bool wireframe : 1 = false;
		BlendFactor sourceColorFactor : 4 = BlendFactor::SourceAlpha;
		BlendFactor sourceAlphaFactor : 4 = BlendFactor::Zero;
		BlendOperation colorBlendOperation : 4 = BlendOperation::Add;
		BlendFactor destColorFactor : 4 = BlendFactor::OneMinusSourceAlpha;
		BlendFactor destAlphaFactor : 4 = BlendFactor::One;
		BlendOperation alphaBlendOperation : 4 = BlendOperation::Add;
		bool operator==(MetaMaterialInfo const& rhs) const { return *reinterpret_cast<uint32_t const*>(this) == reinterpret_cast<uint32_t const&>(rhs); }
	};

	struct PipelineInfo
	{
		SequenceView<DataType const> vertexLayout;
		MetaMaterialInfo metaMaterial;
		uint32_t subpass = 0;
		RenderPass renderPass;
		DescriptorLayout descriptorLayout;
		Shader vertexShader;
		Shader geometryShader;
		Shader fragmentShader;
	};

	class PipelineState
	{
	private:
		VkPipeline m_handle;

	public:
		PipelineState() : m_handle(VK_NULL_HANDLE) {}
		PipelineState(VkPipeline handle) : m_handle(handle) {}
		bool Create(PipelineInfo const& info);
		void Destroy();
		VkPipeline GetHandle() const { return m_handle; }
		bool operator==(PipelineState const& rhs) const { return m_handle == rhs.m_handle; }
	};
}