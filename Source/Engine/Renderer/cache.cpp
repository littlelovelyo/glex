#include "Engine/Renderer/cache.h"
#include "Engine/Renderer/renderer.h"
#include "Core/Platform/filesync.h"
#include "Core/log.h"
#include "Core/Utils/raii.h"
#include "Core/Utils/string.h"
#include "Core/assert.h"

using namespace glex;
using namespace render;

gl::Shader ShaderModuleCache::GetShaderModule(char const* file, void const* bytecode, uint32_t size)
{
	auto iter = m_pathTable.find(file);
	if (iter != m_pathTable.end())
	{
		gl::Shader shader = iter->second;
		m_refCount[shader.GetHandle()].second++;
		return shader;
	}

	char* fileCopy = StringUtils::Copy(file);
	gl::Shader& shader = m_pathTable[fileCopy];
	if (!shader.Create(bytecode, size))
	{
		m_pathTable.erase(fileCopy);
		Mem::Free(fileCopy);
		Logger::Error("Cannot create shader: %s.", file);
		return gl::Shader();
	}
	m_refCount[shader.GetHandle()] = { fileCopy, 1 };
	return shader;
}

void ShaderModuleCache::FreeShaderModule(gl::Shader shader)
{
	auto iter = m_refCount.find(shader.GetHandle());
	auto& [path, refCount] = iter->second;
	if (--refCount == 0)
	{
		m_pathTable.erase(path);
		Mem::Free(path);
		m_refCount.erase(iter);
		Renderer::PendingDelete([=]() mutable
		{
			shader.Destroy();
		});
	}
}

gl::DescriptorSetLayout DescriptorLayoutCache::GetDescriptorSetLayoutInternal(char const* description, SequenceView<gl::DescriptorBinding const> bindings)
{
	auto iter = m_setTable.find(description);
	if (iter != m_setTable.end())
	{
		m_setRefCount[iter->second.GetHandle()].second++;
		return iter->second;
	}

	char* descCopy = StringUtils::Copy(description);
	gl::DescriptorSetLayout& layout = m_setTable[descCopy];
	if (!layout.Create(bindings))
	{
		m_setTable.erase(descCopy);
		Mem::Free(descCopy);
		Logger::Error("Cannot create descriptor set: %s.", description);
		return gl::DescriptorSetLayout();
	}
	m_setRefCount[layout.GetHandle()] = { descCopy, 1 };
	return layout;
}

void DescriptorLayoutCache::FreeDescriptorSetLayout(gl::DescriptorSetLayout layout)
{
	auto iter = m_setRefCount.find(layout.GetHandle());
	auto& [description, refCount] = iter->second;
	GLEX_DEBUG_ASSERT(refCount != 0) {}
	if (--refCount == 0)
	{
		m_setTable.erase(description);
		Mem::Free(description);
		m_setRefCount.erase(iter);
		Renderer::PendingDelete([=]() mutable
		{
			layout.Destroy();
		});
	}
}

uint32_t DescriptorLayoutCache::HashDescriptorSetLayout(SequenceView<gl::DescriptorBinding const> bindings, char* buffer, uint32_t size)
{
	uint32_t ptr = 0;
	for (uint32_t i = 0; i < bindings.Size(); i++)
	{
		gl::DescriptorBinding const& binding = bindings[i];
		if (i != 0)
		{
			if (ptr + 2 > size)
				goto TOO_LONG_ERROR;
			buffer[ptr++] = ',';
		}
		else if (ptr + 1 > size)
			goto TOO_LONG_ERROR;
		switch (binding.type)
		{
			case gl::DescriptorType::Sampler: buffer[ptr++] = 's'; break;
			case gl::DescriptorType::CombinedImageSampler: buffer[ptr++] = 't'; break;
			case gl::DescriptorType::SampledImage: buffer[ptr++] = 'i'; break;
			case gl::DescriptorType::UniformBuffer: buffer[ptr++] = 'u'; break;
			default: std::unreachable();
		}
		uint32_t length = StringUtils::ToString(binding.arraySize, buffer + ptr, size - ptr - 1); // Minus one beforehand.
		if (length == 0)
			goto TOO_LONG_ERROR;
		ptr += length;
		buffer[ptr++] = '@';
		length = StringUtils::ToString(binding.bindingPoint, buffer + ptr, size - ptr);
		if (length == 0)
			goto TOO_LONG_ERROR;
		ptr += length;
		if (binding.shaderStage == gl::ShaderStage::AllGraphics)
		{
			if (ptr + 2 > size)
				goto TOO_LONG_ERROR;
			buffer[ptr++] = 'a';
			buffer[ptr++] = 'g';
		}
		else
		{
			if ((binding.shaderStage & gl::ShaderStage::Vertex) != gl::ShaderStage::None)
			{
				if (ptr + 1 > size)
					goto TOO_LONG_ERROR;
				buffer[ptr++] = 'v';
			}
			if ((binding.shaderStage & gl::ShaderStage::Geometry) != gl::ShaderStage::None)
			{
				if (ptr + 1 > size)
					goto TOO_LONG_ERROR;
				buffer[ptr++] = 'g';
			}
			if ((binding.shaderStage & gl::ShaderStage::Fragment) != gl::ShaderStage::None)
			{
				if (ptr + 1 > size)
					goto TOO_LONG_ERROR;
				buffer[ptr++] = 'f';
			}
		}
	}
	return ptr;
TOO_LONG_ERROR:
	Logger::Error("Descriptor layout is too long.");
	return UINT_MAX;
}

gl::DescriptorSetLayout DescriptorLayoutCache::GetDescriptorSetLayout(SequenceView<gl::DescriptorBinding const> bindings)
{
	char description[Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH + 1];
	uint32_t length = HashDescriptorSetLayout(bindings, description, Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH);
	if (length == UINT_MAX)
		return gl::DescriptorSetLayout();
	description[length] = 0;
	return GetDescriptorSetLayoutInternal(description, bindings);
}

gl::DescriptorLayout DescriptorLayoutCache::GetDescriptorLayout(DescriptorLayoutType const& layout, gl::ShaderStage constantStages, gl::DescriptorSetLayout& outMaterialLayout, gl::DescriptorSetLayout& outObjectLayout)
{
	// Descriptor layout. We need a way to serialize this stuff into a human-readable string so we can hash it. Such as:
	// "0:u1@0vf;2:u1@0f,t1@1f,t1@2f;vf" — "set : type length @ binding stages ; ... ; constant_stages"

	// Push constants is considered 'set 0' when considering compatibility. NO!
	char description[Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH + 1];
	uint32_t ptr = 0;
	for (uint32_t i = 0; i < Limits::NUM_DESCRIPTOR_SETS; i++)
	{
		// Fuck it. I'll use my global layout.
		if (i == Renderer::GLOBAL_DESCRIPTOR_SET)
			continue;
		auto& list = layout[i];
		if (list.empty())
			continue;

		// Since it's just a one-digit number, we don't bother to use the ConvertInteger method.
		static_assert(Limits::NUM_DESCRIPTOR_SETS < 10);
		if (ptr != 0)
		{
			if (ptr + 3 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
			description[ptr++] = ';';
		}
		else
		{
			if (ptr + 2 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
		}
		description[ptr++] = i + '0';
		description[ptr++] = ':';

		uint32_t length = HashDescriptorSetLayout(list, description + ptr, Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH - ptr);
		if (length == UINT_MAX)
			goto TOO_LONG_ERROR;
		ptr += length;
	}
	/*if (constantStages != gl::ShaderStage::None)
	{
		if (ptr != 0)
		{
			if (ptr + 2 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
			description[ptr++] = ';';
		}
		if ((constantStages & gl::ShaderStage::Vertex) != gl::ShaderStage::None)
		{
			if (ptr + 1 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
			description[ptr++] = 'v';
		}
		if ((constantStages & gl::ShaderStage::Geometry) != gl::ShaderStage::None)
		{
			if (ptr + 1 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
			description[ptr++] = 'g';
		}
		if ((constantStages & gl::ShaderStage::Fragment) != gl::ShaderStage::None)
		{
			if (ptr + 1 >= Limits::DESCRIPTOR_LAYOUT_STRING_LENGTH)
				goto TOO_LONG_ERROR;
			description[ptr++] = 'f';
		}
	}*/
	description[ptr] = 0;
	goto NOT_TOO_LONG;
TOO_LONG_ERROR:
	Logger::Error("Descriptor layout is too long.");
	return gl::DescriptorLayout();
NOT_TOO_LONG:

	auto iter = m_descTable.find(description);
	if (iter != m_descTable.end())
	{
		DescriptorLayoutInternal const& layout = iter->second;
		gl::DescriptorLayout pipelineLayout = layout.layout;
		m_refCount[pipelineLayout.GetHandle()].second++;
		outMaterialLayout = layout.sets[Renderer::MATERIAL_DESCRIPOR_SET];
		return pipelineLayout;
	}

	char* descCopy = StringUtils::Copy(description);
	std::array<gl::DescriptorSetLayout, Limits::NUM_DESCRIPTOR_SETS> setLayouts;
	DescriptorLayoutInternal& layoutInternal = m_descTable[descCopy];
	static_assert(Limits::NUM_DESCRIPTOR_SETS < 10);
	char* setBegin = descCopy;
	for (uint32_t i = 0; i < Limits::NUM_DESCRIPTOR_SETS; i++)
	{
		gl::DescriptorSetLayout setLayout;
		// Fuck it. Use our global layout.
		if (i == Renderer::GLOBAL_DESCRIPTOR_SET && (setLayout = Renderer::GetRenderPipeline()->GetGlobalDescriptorSetLayout()).GetHandle() != VK_NULL_HANDLE)
		{
			m_setRefCount[setLayout.GetHandle()].second++;
			/*if (setBegin != nullptr && *setBegin - '0' == i)
			{
				char* setEnd = strchr(setBegin, ';');
				setBegin = setEnd != nullptr ? setEnd + 1 : nullptr;
			}*/
		}
		else if (setBegin == nullptr || *setBegin == 0 || !isdigit(*setBegin))
			break;
		else if (*setBegin - '0' != i)
		{
			// Empty descriptor set.
			GLEX_DEBUG_ASSERT(layout[i].empty()) {}
			setLayout = GetDescriptorSetLayoutInternal("", layout[i]);
		}
		else
		{
			char* setEnd = strchr(setBegin, ';');
			if (setEnd != nullptr)
				*setEnd = 0;
			setLayout = GetDescriptorSetLayoutInternal(setBegin + 2, layout[i]);
			if (setEnd != nullptr)
			{
				*setEnd = ';';
				setBegin = setEnd + 1;
			}
			else
				setBegin = nullptr;
		}
		if (setLayout.GetHandle() == VK_NULL_HANDLE)
		{
			for (uint32_t j = 0; j < i; j++)
			{
				if (layoutInternal.sets[j].GetHandle() != VK_NULL_HANDLE)
					FreeDescriptorSetLayout(layoutInternal.sets[j]);
			}
			m_descTable.erase(descCopy);
			Mem::Free(descCopy);
			return gl::DescriptorLayout();
		}
		layoutInternal.sets[i] = setLayout;
		layoutInternal.numSets = glm::max(layoutInternal.numSets, i + 1);
		setLayouts[i] = setLayout;
		if (i == Renderer::MATERIAL_DESCRIPOR_SET)
			outMaterialLayout = setLayout;
		else if (i == Renderer::OBJECT_DESCRIPTOR_SET)
			outObjectLayout = setLayout;
	}
	if (!layoutInternal.layout.Create(SequenceView(setLayouts.data(), layoutInternal.numSets), gl::ShaderStage::AllGraphics)) // Push constants is considered set 0.
	{
		for (uint32_t i = 0; i < layoutInternal.numSets; i++)
		{
			if (layoutInternal.sets[i] != nullptr)
				FreeDescriptorSetLayout(layoutInternal.sets[i]);
		}
		m_descTable.erase(descCopy);
		Mem::Free(descCopy);
		return gl::DescriptorLayout();
	}
	m_refCount[layoutInternal.layout.GetHandle()] = { descCopy, 1 };
	return layoutInternal.layout;
}

void DescriptorLayoutCache::FreeDescriptorLayout(gl::DescriptorLayout layout)
{
	auto refIter = m_refCount.find(layout.GetHandle());
	auto& [desc, refCount] = refIter->second;
	if (--refCount == 0)
	{
		auto iter = m_descTable.find(desc);
		DescriptorLayoutInternal& layoutInternal = iter->second;
		for (uint32_t i = 0; i < layoutInternal.numSets; i++)
			FreeDescriptorSetLayout(layoutInternal.sets[i]);
		Renderer::PendingDelete([=]() mutable
		{
			layout.Destroy();
		});
		m_descTable.erase(iter);
		Mem::Free(desc);
		m_refCount.erase(refIter);
	}
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		Pipeline state.
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
gl::PipelineState PipelineStateCache::GetPipelineState(WeakPtr<Shader> shader, gl::MetaMaterialInfo metaMaterial, gl::RenderPass renderPass, uint32_t subpass)
{
	PipelineStateKey key;
	key.renderPass = renderPass;
	key.subpass = subpass;
	key.shader = shader;
	key.metaMaterial = metaMaterial;

	gl::PipelineState& ret = m_pipelineStates[key];
	if (ret.GetHandle() != VK_NULL_HANDLE)
	{
		m_refCounts[ret.GetHandle()].second++;
		return ret;
	}
	gl::PipelineInfo info;
	info.vertexLayout = shader->GetVertexLayout();
	info.metaMaterial = metaMaterial;
	info.renderPass = renderPass;
	info.subpass = subpass;
	info.descriptorLayout = shader->GetDescriptorLayout();
	info.vertexShader = shader->GetVertexShader();
	info.geometryShader = shader->GetGeometryShader();
	info.fragmentShader = shader->GetFragmentShader();
	if (!ret.Create(info))
	{
		m_pipelineStates.erase(key);
		return gl::PipelineState();
	}

	m_refCounts[ret.GetHandle()] = { key, 1 };
	return ret;
}

void PipelineStateCache::FreePipelineState(gl::PipelineState pipelineState)
{
	auto& [key, refCount] = m_refCounts[pipelineState.GetHandle()];
	GLEX_DEBUG_ASSERT(refCount != 0) {}
	if (--refCount == 0)
	{
		m_refCounts.erase(pipelineState.GetHandle());
		m_pipelineStates.erase(key);
		Renderer::PendingDelete([=]() mutable
		{
			pipelineState.Destroy();
		});
	}
}