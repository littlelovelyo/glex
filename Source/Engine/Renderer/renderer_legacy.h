/**
 * Renderer.
 *
 * It is responsible for creating rendering thread, which
 * does all the actual OpenGL calls.
 *
 * It is also used for rendering a frame. The renderer holds
 * a pipeline object, which does forward rendering or deferred rendering.
 */
#pragma once
#include "config.h"
#include "Context/app.h"
#include "Graphics/vertex_layout.h"
#include "Graphics/frame_buffer.h"
#include "Graphics/buffer.h"
#include "Memory/framealloc.h"
#include "Thread/thread.h"
#include "Utils/optional.h"
#include "Utils/ringbuffer.h"
#include "Resource/pointer.h"
#include "Resource/resmgr.h"
#include "Component/scene.h"
#include "pipeline.h"

namespace glex
{

namespace impl
{

template <typename T>
class ParameterPlaceHolder
{
private:
	T m_value;

public:
	template <typename R> R& operator=(R&& value) { return lval_cast<std::remove_cv_t<std::remove_reference_t<R>>>(m_value) = std::forward<R>(value); }
	template <typename R> R& As() { return lval_cast<R>(m_value); }
	template <typename R> std::pair<R, R>& AsPair() { return As<std::pair<R, R>>(); }
	template <typename R> Pointer<R>& AsPointer() { return As<Pointer<R>>(); }
	template <typename R> RefCountedResource<R>*& AsResource() { return As<RefCountedResource<R>*>(); }
	template <typename R> ResPtr<R>& AsResPtr() { return As<ResPtr<R>>(); }
};

struct RenderCommand
{
	uint32_t command;
	ParameterPlaceHolder<uint32_t> flags;
	ParameterPlaceHolder<uint64_t> param;
	ParameterPlaceHolder<uint64_t> in;
	ParameterPlaceHolder<uint64_t> out;

	enum Command : uint32_t
	{
		Exit,
		ViewPort,
		Enable,
		Disable,
		BlendFunc,
		ClearColor,
		ClearDepth,
		ColorMask,
		DepthMask,
		Draw,
		DrawWithIndexCount,
		DrawLinesWithIndexCount,
		EndFrame,
		SwapBuffers,
		EnableVSync,
		DisableVSync,

		CreateStaticBuffer,
		CreateDynamicBuffer,
		DestroyBuffer,
		ReallocateBuffer,
		UpdateBuffer,
		BindBuffer,
		BindBufferBase,

		CreateFrameBuffer,
		DestroyFrameBuffer,
		ResizeFrameBuffer,
		AddTextureColorAttachment,
		AddMultisampleTextureColorAttachment,
		AddRenderBufferColorAttachment,
		AddMultisampleRenderBufferColorAttachment,
		AddTextureDepthAttachment,
		AddTextureArrayDepthAttachment,
		AddRenderBufferDepthAttachment,
		AddMultisampleRenderBufferDepthAttachment,
		BindFrameBuffer,
		BindReadFrameBuffer,
		BindDrawFrameBuffer,
		UnbindFrameBuffer,
		UnbindReadFrameBuffer,
		UnbindDrawFrameBuffer,
		BlitFrameBuffer,

		CreateShader,
		DestroyShader,
		BindShader,
		DispatchCompute,
		SetShaderUniform1ui,
		CreateTexture,
		CreateTextureArray,
		DestroyTexture,
		BindTexture,
		CreateMesh,
		CreateEmptyMesh,
		DestroyMesh,
		BindMesh,
		LoadMaterial,
		CreateMaterial,
		DestroyMaterial,
		BindMaterial,
		SetMaterialTextureByIndex,
		SetMaterialTextureByName,
		SetMaterialFloatByName,
		SetMaterialVec3ByName
	};

	enum BufferTarget : uint32_t
	{
		VertexBuffer = GL_ARRAY_BUFFER,
		IndexBuffer = GL_ELEMENT_ARRAY_BUFFER,
		UniformBuffer = GL_UNIFORM_BUFFER,
		ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER
	};

	enum Function : uint32_t
	{
		DepthTest = GL_DEPTH_TEST,
		StencilTest = GL_STENCIL_TEST,
		Blending = GL_BLEND,
		FaceCulling = GL_CULL_FACE
	};

	enum BlendingFactor : uint32_t
	{
		Zero = GL_ZERO,
		One = GL_ONE,
		SourceColor = GL_SRC_COLOR,
		OneMinusSourceColor = GL_ONE_MINUS_SRC_COLOR,
		DestinationColor = GL_DST_COLOR,
		OneMinusDestinationColor = GL_ONE_MINUS_DST_COLOR,
		SourceAlpha = GL_SRC_ALPHA,
		OneMinusSourceAlpha = GL_ONE_MINUS_SRC_ALPHA,
		DestinationAlpha = GL_DST_ALPHA,
		OneMinusDestinationAlpha = GL_ONE_MINUS_DST_ALPHA
	};
};

}

class Renderer
{
	Renderer() = delete;

private:
	inline static Event* s_frameEndEvent, *s_newCommandEvent;
	static RingBuffer<impl::RenderCommand> s_commandBuffer;
	inline static Optional<Thread> s_renderingThread;
	inline static Pipeline* s_pipeline;

	struct QuadVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
		uint16_t textureID;
		uint16_t flags;
		glm::vec4 color;
		union
		{
			struct
			{
				glm::vec3 secondaryColor;
				glm::vec2 offset;
				glm::vec2 border;
			} sdf;
			struct
			{
				float umin, vmin, ulength, vlength;
				float textureBorder, screenBorderX, screenBorderY;
			} slice;
		};
	};

	inline static ResPtr<Texture> s_textureBound[32];
	inline static ResPtr<Mesh> s_quadMesh;
	inline static ResPtr<Shader> s_quadShader;
	inline static QuadVertex* s_quadVertexBuffer;
	inline static uint32_t* s_quadIndexBuffer;
	inline static uint32_t s_currentQuadCount;

	static uint16_t BindTexture(ResPtr<Texture> texture);
	static void Flush();
	static void RenderProcess(Thread* thread);

public:
	static void Render(Scene& scene) { s_pipeline->Render(scene); }
	static void BeginUI();
	static void EndUI();
	static void DrawControl(Control* control);
	static void DrawQuad(glm::vec4 const& border, glm::vec4 const& color);
	static void DrawQuad(glm::vec4 const& border, ResPtr<Texture> texture, glm::vec4 uv, glm::vec4 const& color);
	static void EndFrame();
#if GLEX_DEBUG_RENDERING
	static void DrawDebugLine(glm::vec3 const* begin, glm::vec3 const* end, glm::vec3 const& color) { s_pipeline->DrawDebugLine(begin, end, color); }
	static void DrawDebugSphere(glm::vec3 const& origin, float radius, glm::vec3 const& color);
#endif

#ifdef GLEX_INTERNAL
	static void Startup(Pipeline* (*pipeline)(), char const* quadShaderPath, bool vsync);
	static void Shutdown();
	static void OnResize(uint32_t width, uint32_t height) { s_pipeline->OnResize(width, height); }

	// Render commands.
	static void SetViewPort(uint32_t width, uint32_t height);
	static void SetFunctionEnabled(impl::RenderCommand::Function, bool enabled);
	static void SetBlendingFunction(impl::RenderCommand::BlendingFactor srcColor, impl::RenderCommand::BlendingFactor dstColor, impl::RenderCommand::BlendingFactor srcAlpha, impl::RenderCommand::BlendingFactor dstAlpha);
	static void ClearColor(uint32_t i, float r, float g, float b, float a); // Set i to -1 to clear all buffers.
	static void ClearDepth(float depth);
	static void ColorMask(bool r, bool g, bool b, bool a);
	static void DepthMask(bool d);
	static void Draw();
	static void Draw(uint32_t indexCount);
	static void DrawLines(uint32_t indexCount);
	static void SwapBuffers();
	static void SetVSync(bool enabled);

	// Internal resources.
	static void CreateBuffer(impl::GlBuffer* buffer, uint32_t size);	// Create with dynamic data.
	static void CreateBuffer(impl::GlBuffer* buffer, Pointer<const void> data, uint32_t size);	// Create with static data.
	static void DestroyBuffer(impl::GlBuffer* buffer);
	static void ReallocateBuffer(impl::GlBuffer* buffer, uint32_t size);	// Dynamic reallocate.
	static void UpdateBuffer(impl::GlBuffer* buffer, uint32_t offset, Pointer<const void> data, uint32_t size);
	static void BindBuffer(impl::GlBuffer const* buffer, impl::RenderCommand::BufferTarget target);
	static void BindBufferBase(impl::GlBuffer const* buffer, impl::RenderCommand::BufferTarget target, uint32_t index);
	static void CreateFrameBuffer(uint32_t width, uint32_t height, impl::FrameBuffer* out);
	static void DestroyFrameBuffer(impl::FrameBuffer* frameBuffer);
	static void ResizeFrameBuffer(impl::FrameBuffer* frameBuffer, uint32_t width, uint32_t height);
	static void AddColorAttachment(impl::FrameBuffer* frameBuffer, uint32_t slot, Pointer<const TextureFormat> internalFormat);
	static void AddColorAttachment(impl::FrameBuffer* frameBuffer, uint32_t slot, Pointer<const TextureFormat> internalFormat, uint32_t msaaSamples);
	static void AddColorAttachment(impl::FrameBuffer* frameBuffer, uint32_t slot, uint32_t rboFormat);
	static void AddColorAttachment(impl::FrameBuffer* frameBuffer, uint32_t slot, uint32_t rboFormat, uint32_t msaaSamples);
	static void AddDepthAttachment(impl::FrameBuffer* frameBuffer, uint32_t rboFormat);
	static void AddDepthAttachment(impl::FrameBuffer* frameBuffer, uint32_t rboFormat, uint32_t msaaSamples);
	static void AddDepthAttachment(impl::FrameBuffer* frameBuffer, Pointer<const TextureFormat> internalFormat);
	static void AddDepthAttachment(impl::FrameBuffer* frameBuffer, uint32_t size, Pointer<const TextureFormat> internalFormat);
	static void BindFrameBuffer(impl::FrameBuffer* frameBuffer);
	static void BindReadFrameBuffer(impl::FrameBuffer* frameBuffer);
	static void BindDrawFrameBuffer(impl::FrameBuffer* frameBuffer);
	static void UnbindFrameBuffer();
	static void UnbindReadFrameBuffer();
	static void UnbindDrawFrameBuffer();
	static void BlitFrameBuffer(uint32_t width, uint32_t height);
#endif

	// Shaders.
	static void CreateShader(Pointer<const char> src, RefCountedResource<Shader>* out);
	static void DestroyShader(RefCountedResource<Shader>* shader);
	static void BindShader(ResPtr<Shader> shader);
	static void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
	static void SetShaderUniform(ResPtr<Shader> shader, uint32_t location, uint32_t value);

	// Textures.
	static void CreateTexture(Pointer<const uint8_t> data, uint32_t dataFormat, Pointer<const TextureFormat> internalFormat, RefCountedResource<Texture>* out);
	static void CreateTextureArray(uint32_t size, Pointer<const TextureFormat> internalFormat, RefCountedResource<Texture>* out);
	static void DestroyTexture(RefCountedResource<Texture>* texture);
	static void BindTexture(Texture const* texture, uint32_t slot);

	// Meshes.
	static void CreateMesh(Pointer<const char> layout, Pointer<const void> vertexData, uint32_t vertexSize, Pointer<const void> indexData, uint32_t indexSize, glm::vec4 const& boundingShpere, RefCountedResource<Mesh>* out);
	static void CreateMesh(Pointer<const char> layout, uint32_t vertexSize, uint32_t indexSize, RefCountedResource<Mesh>* out);
	static void DestroyMesh(RefCountedResource<Mesh>* mesh);
	static void BindMesh(ResPtr<Mesh> mesh);

	// Materials.
	static void CreateMaterial(ResPtr<Shader> geometryShader, ResPtr<Shader> shadowShader, ResPtr<Shader> colorShader, uint32_t uniformBufferSize, Pointer<const void> uniformBufferData, uint32_t textureCount, Pointer<ResPtr<Texture>> textures, RefCountedResource<Material>* out);
	static void CreateMaterial(ResPtr<Shader> geometryShader, ResPtr<Shader> shadowShader, ResPtr<Shader> colorShader, RefCountedResource<Material>* out);
	static void DestroyMaterial(RefCountedResource<Material>* material);
	static void BindMaterial(ResPtr<Material> material, RenderPass pass);
	static void SetMaterialTexture(ResPtr<Material> material, uint32_t index, ResPtr<Texture> texture);
	static void SetMaterialTexture(ResPtr<Material> material, Pointer<const char> name, ResPtr<Texture> texture);
	static void SetMaterialFloat(ResPtr<Material> material, Pointer<const char> name, float value);
	static void SetMaterialVec3(ResPtr<Material> material, Pointer<const char> name, glm::vec3 const& value);
};

};