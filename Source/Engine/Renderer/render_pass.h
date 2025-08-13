/**
 * TODO: add memory alias support.
 */
#pragma once
#include "Core/GL/render_pass.h"
#include "Core/GL/frame_buffer.h"
#include "Core/GL/command.h"
#include "Core/Container/basic.h"
#include "Core/assert.h"
#include "Engine/Renderer/image.h"
#include "Engine/Renderer/mesh.h"
#include "Engine/Renderer/matinst.h"

namespace glex
{
	namespace py
	{
		class RenderPassBuilder;
		class RenderPass;
	}

	class RenderPass : private Unmoveable
	{
		friend class py::RenderPassBuilder;
		friend class py::RenderPass;

	protected:
		constexpr static gl::AttachmentLoadOperation LOAD_OPERATION_UNSET = static_cast<gl::AttachmentLoadOperation>(0xff);
		constexpr static gl::AttachmentStoreOperation STORE_OPERATION_UNSET = static_cast<gl::AttachmentStoreOperation>(0xff);

		struct AttachmentInternal
		{
			WeakPtr<ImageView> attachment;
			gl::ImageFormat format;
			uint8_t samples;
			gl::AttachmentLoadOperation firstLoadOp = LOAD_OPERATION_UNSET;
			gl::AttachmentStoreOperation lastStoreOp = STORE_OPERATION_UNSET;
			gl::ImageLayout initialLayout;
			gl::ImageLayout finalLayout;
			uint32_t lastReadSubpass = UINT_MAX;
			uint32_t lastWriteSubpass = UINT_MAX;
		};

		struct SubpassInternal
		{
			Vector<std::pair<uint32_t, gl::ImageLayout> const> inputs;
			Vector<std::pair<uint32_t, gl::ImageLayout> const> colorOutputs;
			std::pair<uint32_t, gl::ImageLayout> depthStencilOutputs = { UINT_MAX, gl::ImageLayout::Undefined };
			Vector<uint32_t> passThroughs;
		};

		class Builder
		{
		private:
			glm::uvec2 m_renderAera = glm::uvec2(0.0f, 0.0f);
			HashMap<WeakPtr<ImageView>, uint32_t> m_attachmentIndices;
			Vector<AttachmentInternal> m_attachments;
			Vector<SubpassInternal> m_subpasses;
			Vector<gl::DependencyInfo> m_dependencies;

			uint32_t GetOrAddAttachment(WeakPtr<ImageView> attachment);

		public:
			void PushSubpass();
			void Read(WeakPtr<ImageView> attachment);
			void Write(WeakPtr<ImageView> attachment);
			void Clear(WeakPtr<ImageView> attachment);
			void Output(WeakPtr<ImageView> attachment);
			Vector<AttachmentInternal>& GetAttachments() { return m_attachments; }
			Vector<AttachmentInternal> const& GetAttachments() const { return m_attachments; }
			Vector<SubpassInternal> const& GetSubpasses() const { return m_subpasses; }
			Vector<gl::DependencyInfo> const& GetDependencies() const { return m_dependencies; }
			glm::uvec2 GetRenderAera() const { return m_renderAera; }
		};

		// END OF NESTED CLASS DEFINITION

		// STATIC MEMBERS
		inline static HashMap<SequenceView<uint32_t>, gl::RenderPass> s_renderPassCache;
		inline static HashMap<VkRenderPass, std::pair<SequenceView<uint32_t>, uint32_t>> s_refCounts;
		static gl::RenderPass GetCachedOrCreateNewRenderPass(Builder const& builder);
		static void FreeRenderPass(gl::RenderPass renderPass);

#if GLEX_REPORT_MEMORY_LEAKS
	public:
		static void FreeMemory()
		{
			decltype(s_renderPassCache) x;
			decltype(s_refCounts) y;
			s_renderPassCache.swap(x);
			s_refCounts.swap(y);
		}
	protected:
#endif

		struct AttachmentInformation
		{
			WeakPtr<ImageView> imageView;
			gl::ImageLayout initialLayout;
			gl::ImageLayout finalLayout;
		};

		// INSTANCE MEMBERS
		gl::RenderPass m_renderPassObject;
		gl::FrameBuffer m_frameBuffer;
		glm::vec2 m_renderAera;
		Vector<AttachmentInformation> m_attachments;

	protected:
		Builder BeginRenderPassDefinition()
		{
			GLEX_DEBUG_ASSERT(m_renderPassObject.GetHandle() == VK_NULL_HANDLE) {}
			return Builder();
		}

		bool EndRenderPassDefinition(Builder& builder);
		void BeginRenderPass(SequenceView<gl::ClearValue const> clearValues);
		void NextSubpass();
		void EndRenderPass();
		void BindMaterial(WeakPtr<MaterialInstance> material) { material->Bind(); }
		void BindObjectData(void const* data, uint32_t size);
		void DrawMesh(WeakPtr<Mesh> mesh) { mesh->Draw(); }
		void DrawAllControls();

	public:
		~RenderPass();
		bool IsValid() const { return m_frameBuffer.GetHandle() != VK_NULL_HANDLE; }
		void Invalidate();
		gl::RenderPass GetRenderPassObject() const { return m_renderPassObject; }
		bool Recreate();
	};
}