/**
 * Shader module and descriptor layout should be cached so we don't
 * create them twice and they can destroyed properly.
 *
 * TODO: make them (and all other resource management methods) thread-safe.
 */
#pragma once
#include "Core/Container/basic.h"
#include "Core/GL/shader.h"
#include "Core/GL/descriptor.h"
#include "Core/assert.h"
#include "Core/Memory/smart_ptr.h"
#include "Engine/Renderer/shader.h"
#include <array>

namespace glex::render
{
	class ShaderModuleCache
	{
	private:
		HashMap<char const*, gl::Shader, eastl::hash<char const*>, eastl::str_equal_to<char const*>> m_pathTable;
		HashMap<VkShaderModule, std::pair<char*, uint32_t>> m_refCount;

	public:
		gl::Shader GetShaderModule(char const* file, void const* bytecode, uint32_t size);
		void FreeShaderModule(gl::Shader shader);

#if GLEX_REPORT_MEMORY_LEAKS
		void FreeMemory()
		{
			GLEX_DEBUG_ASSERT(m_pathTable.empty()) {}
			GLEX_DEBUG_ASSERT(m_refCount.empty()) {}
			decltype(m_pathTable) x;
			decltype(m_refCount) y;
			m_pathTable.swap(x);
			m_refCount.swap(y);
		}
#endif
	};

	struct DescriptorLayoutInternal
	{
		std::array<gl::DescriptorSetLayout, Limits::NUM_DESCRIPTOR_SETS> sets;
		gl::DescriptorLayout layout;
		uint32_t numSets;
	};

	class DescriptorLayoutCache
	{
	private:
		// It's stupid to keep a name inside a class. But here we could use string since it's internal.
		HashMap<char const*, gl::DescriptorSetLayout, eastl::hash<char const*>, eastl::str_equal_to<char const*>> m_setTable;
		HashMap<VkDescriptorSetLayout, std::pair<char*, uint32_t>> m_setRefCount;

		HashMap<char const*, DescriptorLayoutInternal, eastl::hash<char const*>, eastl::str_equal_to<char const*>> m_descTable;
		HashMap<VkPipelineLayout, std::pair<char*, uint32_t>> m_refCount;

		gl::DescriptorSetLayout GetDescriptorSetLayoutInternal(char const* description, SequenceView<gl::DescriptorBinding const> bindings);
		uint32_t HashDescriptorSetLayout(SequenceView<gl::DescriptorBinding const> bindings, char* buffer, uint32_t size);

	public:
		gl::DescriptorSetLayout GetDescriptorSetLayout(SequenceView<gl::DescriptorBinding const> bindings);
		void FreeDescriptorSetLayout(gl::DescriptorSetLayout layout);
		gl::DescriptorLayout GetDescriptorLayout(DescriptorLayoutType const& layout, gl::ShaderStage constantStages, gl::DescriptorSetLayout& outMaterialLayout, gl::DescriptorSetLayout& outObjectLayout);
		void FreeDescriptorLayout(gl::DescriptorLayout layout);

#if GLEX_REPORT_MEMORY_LEAKS
		void FreeMemory()
		{
			GLEX_DEBUG_ASSERT(m_setTable.empty()) {}
			GLEX_DEBUG_ASSERT(m_descTable.empty()) {}
			GLEX_DEBUG_ASSERT(m_refCount.empty()) {}
			GLEX_DEBUG_ASSERT(m_setRefCount.empty()) {}
			decltype(m_setTable) x;
			decltype(m_descTable) y;
			decltype(m_refCount) z;
			decltype(m_setRefCount) w;
			m_setTable.swap(x);
			m_descTable.swap(y);
			m_refCount.swap(z);
			m_setRefCount.swap(w);
		}
#endif
	};

	class PipelineStateCache
	{
	private:
		struct PipelineStateKey
		{
			gl::RenderPass renderPass;
			uint32_t subpass;
			gl::MetaMaterialInfo metaMaterial;
			WeakPtr<Shader> shader;
			bool operator==(PipelineStateKey const& rhs) const = default;
		};

		class Hasher
		{
		public:
			size_t operator()(PipelineStateKey const& key) const
			{
				uint64_t shader = reinterpret_cast<uint64_t>(key.shader.Get());
				uint64_t metaMaterial = reinterpret_cast<uint32_t const&>(key.metaMaterial);
				uint64_t subpass = key.subpass;
				uint64_t renderPass = reinterpret_cast<uint64_t>(key.renderPass.GetHandle());
				return shader * 307 + renderPass * 169 + (metaMaterial << 32) ^ subpass;
			}
		};

		HashMap<PipelineStateKey, gl::PipelineState, Hasher> m_pipelineStates;
		HashMap<VkPipeline, std::pair<PipelineStateKey, uint32_t>> m_refCounts;

	public:
		gl::PipelineState GetPipelineState(WeakPtr<Shader> shader, gl::MetaMaterialInfo metaMaterial, gl::RenderPass renderPass, uint32_t subpass);
		void FreePipelineState(gl::PipelineState pipelineState);

#if GLEX_REPORT_MEMORY_LEAKS
		void FreeMemory()
		{
			GLEX_DEBUG_ASSERT(m_pipelineStates.empty()) {}
			GLEX_DEBUG_ASSERT(m_refCounts.empty()) {}
			decltype(m_pipelineStates) x;
			decltype(m_refCounts) y;
			m_pipelineStates.swap(x);
			m_refCounts.swap(y);
		}
#endif
	};
}