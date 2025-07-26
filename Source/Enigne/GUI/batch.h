#pragma once
/**
 * Batching Algorithm:
 * 1. Sort chilren of the same level (they have the children of an element) based on their Z order.
 * 2. Recursively add their children into draw list.
 * 3. Select the first element (which is at the bottom of the UI stack) and go through its children,
 *    adding batchable elements recursively.
 * 4. Upon encountering the first unbatchable child of an element, break the loop and start going
 *    through it siblings.
 * 5. When a batch is sent to renderer, remove it from the list.
 *
 * Optimization:
 * We don't add all of the UI elements in one go. Whenever we encounter an unbatchable sibling,
 * we first batch them recursively and send them to renderer, which may save some memory and improve caching.
 */
#include "Core/Container/basic.h"
#include "Core/GL/buffer.h"
#include "Core/GL/render_pass.h"
#include "Core/GL/frame_buffer.h"
#include "Engine/Renderer/buffer.h"
#include "Engine/Renderer/texture.h"
#include "Engine/Renderer/descmgr.h"
#include "Engine/GUI/control.h"

namespace glex::ui
{
	struct QuadVertex
	{
		glm::vec2 pos;
		uint32_t texID;
		glm::vec2 uv;
		glm::vec4 color;
	};

	struct DrawElement
	{
		WeakPtr<Control> control;
		uint32_t childIndex;
		uint32_t groupSize;
		uint32_t nextGroup;
	};

	class BatchRenderer : private StaticClass
	{
	private:
		inline static gl::DescriptorSet s_emptySet;
		inline static gl::DescriptorSetLayout s_textureSetLayout;
		inline static Optional<render::DynamicDescriptorAllocator> s_textureDescriptorAllocator;

		// Vertex/index buffers.
		inline static Optional<Buffer> s_vertexBuffer, s_indexBuffer;
		inline static QuadVertex* s_vertexMap;
		inline static uint32_t* s_indexMap;
		inline static uint32_t s_vertexBegin, s_indexBegin;
		inline static uint32_t s_vertexEnd, s_indexEnd;

		// Textures.
		inline static uint32_t s_availableTextures;
		inline static uint32_t s_numTextures;
		inline static Vector<WeakPtr<Texture>> s_textures;

		inline static Vector<DrawElement> s_drawList;
		inline static Vector<gl::ImageSamplerDesciptor> s_descriptors;
		inline static WeakPtr<MaterialInstance> s_currentBatchMaterial;

		// Hit test.
		inline static glm::vec2 s_hoverControlPos;
		inline static WeakPtr<Control> s_hoverControlLastFrame;
		inline static WeakPtr<Control> s_hoverControlThisFrame;
		inline static WeakPtr<Control> s_activeControl;
		inline static double s_doubleClickTime;

		inline static uint32_t AddChildrenToDrawList(WeakPtr<Control> control);
		inline static void Flush();
		inline static void UseMaterial(WeakPtr<MaterialInstance> material);
		inline static void PushMeshData(SequenceView<QuadVertex const> vertexData, SequenceView<uint32_t const> indexData, WeakPtr<Texture> texture);
		inline static void BuildBatch(uint32_t rootIndex, glm::vec2 parentPosition);

	public:
		static bool Startup(uint32_t initialQuadBudget);
		static void Shutdown();
		static void Tick();
		static void BeginUIPass();
		static void PaintControl(WeakPtr<Control> control);
		static void PaintPopup(WeakPtr<Control> control, glm::vec2 pos);
		static void EndUIPass();
		static void DrawQuad(glm::vec4 const& border, WeakPtr<Texture> texture, glm::vec4 const& uv, glm::vec4 const& color);
		static void SetFocus(WeakPtr<Control> control);
		static WeakPtr<Control> GetFocus() { return s_activeControl; }
	};
}