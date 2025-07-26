/**
 * Render pipeline.
 *
 * Render pass is a pain in the ass when dealing with Vulkan.
 * We really can't create a render pass that suits all senarios, so we let the user to decide.
 */
#pragma once
#include "Core/commdefs.h"
#include "Core/GL/context.h"
#include "Core/GL/render_pass.h"
#include "Core/Memory/smart_ptr.h"
#include "Engine/Renderer/render_pass.h"
#include "Engine/Renderer/buffer.h"
#include "Engine/Renderer/texture.h"
#include "Engine/ECS/scene.h"

namespace glex
{
	struct ShaderResource
	{
		gl::DescriptorType type;
		union
		{
			WeakPtr<Buffer> buffer;
			WeakPtr<Texture> texture;
		};

		ShaderResource(WeakPtr<Buffer> buffer) { type = gl::DescriptorType::UniformBuffer, this->buffer = buffer; }
		ShaderResource(WeakPtr<Texture> texture) { type = gl::DescriptorType::CombinedImageSampler, this->texture = texture; }
	};

	class Pipeline : private Unmoveable
	{
	private:
		gl::DescriptorPool m_globalDescriptorPool;
		gl::DescriptorSetLayout m_globalDescriptorSetLayout;
		gl::DescriptorSet m_globalDescriptorSet;

	protected:
		bool BindGlobalData(SequenceView<ShaderResource const> resources);
		void SetGlobalData(WeakPtr<Buffer> buffer, void const* data, uint32_t size);
		glm::uvec2 GetRenderSize() const { return gl::Context::Size(); }

	public:
		void BaseShutdown();

		gl::DescriptorSetLayout GetGlobalDescriptorSetLayout() const { return m_globalDescriptorSetLayout; }
		gl::DescriptorSet GetGlobalDescriptorSet() const { return m_globalDescriptorSet; }
		virtual bool Startup() { return true; }
		virtual void Shutdown() {}
		virtual bool Resize(uint32_t width, uint32_t height) { return true; }
		virtual WeakPtr<ImageView> Render(WeakPtr<Scene> scene) = 0;
		virtual std::tuple<RenderPass const&, uint32_t, gl::MetaMaterialInfo> ResolveMaterialDomain(uint32_t materialDomain) const = 0;
	};
}