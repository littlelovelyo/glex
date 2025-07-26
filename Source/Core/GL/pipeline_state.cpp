#include "Core/GL/pipeline_state.h"
#include "Core/GL/context.h"

using namespace glex::gl;

bool PipelineState::Create(PipelineInfo const& info)
{
	// Dynamic state.
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = std::size(dynamicStates);
	dynamicStateInfo.pDynamicStates = dynamicStates;

	// Vertex layout.
	Vector<VkVertexInputAttributeDescription> vertexAttributes(info.vertexLayout.Size());
	uint32_t offset = 0;
	for (uint32_t i = 0; i < info.vertexLayout.Size(); i++)
	{
		vertexAttributes[i].location = i;
		vertexAttributes[i].binding = 0;
		vertexAttributes[i].format = VulkanEnum::GetDataFormat(info.vertexLayout[i]);
		vertexAttributes[i].offset = offset;
		switch (info.vertexLayout[i])
		{
			case DataType::Float: case DataType::Int: case DataType::UInt:
			offset += 4; break;
			case DataType::Vec2: case DataType::IVec2: case DataType::UVec2:
			offset += 8; break;
			case DataType::Vec3: case DataType::IVec3: case DataType::UVec3:
			offset += 12; break;
			case DataType::Vec4: case DataType::IVec4: case DataType::UVec4:
			offset += 16; break;
			default: Logger::Fatal("Invalid format value!");
		}
	}
	VkVertexInputBindingDescription vertexBinding = {};
	vertexBinding.binding = 0;
	vertexBinding.stride = offset;
	vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkPipelineVertexInputStateCreateInfo vertexInfo = {};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (info.vertexLayout.Size() != 0)
	{
		vertexInfo.vertexBindingDescriptionCount = 1;
		vertexInfo.pVertexBindingDescriptions = &vertexBinding;
	}
	vertexInfo.vertexAttributeDescriptionCount = vertexAttributes.size();
	vertexInfo.pVertexAttributeDescriptions = vertexAttributes.data();

	// Primitive type.
	VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
	assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assemblyInfo.primitiveRestartEnable = VK_FALSE;

	// Viewport.
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;

	// Rasterizer.
	bool wireframe = info.metaMaterial.wireframe && Context::DeviceInfo().SupportsWireframeRendering();
	VkPipelineRasterizationStateCreateInfo rasterInfo = {};
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.depthClampEnable = VK_FALSE;
	rasterInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterInfo.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterInfo.cullMode = VulkanEnum::GetCullMode(info.metaMaterial.cullMode);
	rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterInfo.depthBiasEnable = VK_FALSE;
	// rasterInfo.lineWidth = info.lineWidth; // Auto-clamp.

	// MSAA: change this later.
	uint8_t msaaLevel = 1;
	VkPipelineMultisampleStateCreateInfo msaaInfo = {};
	msaaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	if (msaaLevel < 2)
	{
		msaaInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		msaaInfo.sampleShadingEnable = VK_FALSE;
	}

	// Depth and stencil test.
	VkPipelineDepthStencilStateCreateInfo depthInfo = {};
	depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthInfo.depthTestEnable = info.metaMaterial.depthTest;
	depthInfo.depthWriteEnable = info.metaMaterial.depthWrite;
	depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthInfo.depthBoundsTestEnable = VK_FALSE;
	depthInfo.stencilTestEnable = VK_FALSE;

	// Blending.
	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.blendEnable = info.metaMaterial.blend;
	blendState.srcColorBlendFactor = VulkanEnum::GetBlendFactor(info.metaMaterial.sourceColorFactor);
	blendState.dstColorBlendFactor = VulkanEnum::GetBlendFactor(info.metaMaterial.destColorFactor);
	blendState.colorBlendOp = VulkanEnum::GetBlendOperation(info.metaMaterial.colorBlendOperation);
	blendState.srcAlphaBlendFactor = VulkanEnum::GetBlendFactor(info.metaMaterial.sourceAlphaFactor);
	blendState.dstAlphaBlendFactor = VulkanEnum::GetBlendFactor(info.metaMaterial.destAlphaFactor);
	blendState.alphaBlendOp = VulkanEnum::GetBlendOperation(info.metaMaterial.alphaBlendOperation);
	blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.attachmentCount = 1;
	blendInfo.pAttachments = &blendState;

	// Shaders.
	uint32_t numShaders = 0;
	std::array<VkPipelineShaderStageCreateInfo, 3> shaderInfo = {};
	if (info.vertexShader.GetHandle() != VK_NULL_HANDLE)
	{
		shaderInfo[numShaders].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderInfo[numShaders].module = info.vertexShader.GetHandle();
		numShaders++;
	}
	if (info.geometryShader.GetHandle() != VK_NULL_HANDLE)
	{
		shaderInfo[numShaders].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		shaderInfo[numShaders].module = info.geometryShader.GetHandle();
		numShaders++;
	}
	if (info.fragmentShader.GetHandle() != VK_NULL_HANDLE)
	{
		shaderInfo[numShaders].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderInfo[numShaders].module = info.fragmentShader.GetHandle();
		numShaders++;
	}
	for (uint32_t i = 0; i < numShaders; i++)
	{
		shaderInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo[i].pName = "main";
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = numShaders;
	pipelineInfo.pStages = shaderInfo.data();
	pipelineInfo.pVertexInputState = &vertexInfo;
	pipelineInfo.pInputAssemblyState = &assemblyInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pRasterizationState = &rasterInfo;
	pipelineInfo.pMultisampleState = &msaaInfo;
	pipelineInfo.pDepthStencilState = &depthInfo;
	pipelineInfo.pColorBlendState = &blendInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = info.descriptorLayout.GetHandle();
	pipelineInfo.renderPass = info.renderPass.GetHandle();
	pipelineInfo.subpass = info.subpass;
	if (vkCreateGraphicsPipelines(Context::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, Context::HostAllocator(), &m_handle) == VK_SUCCESS)
		return true;
	m_handle = VK_NULL_HANDLE;
	return false;
}

void PipelineState::Destroy()
{
	vkDestroyPipeline(Context::GetDevice(), m_handle, Context::HostAllocator());
}