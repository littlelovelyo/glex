#include "renderer.h"
#if GLEX_COMMON_LOGGING || GLEX_REPORT_RESOURCE_ERRORS
#include <stdio.h>
#endif

using namespace glex;
using namespace glex::impl;

void Renderer::Startup(Pipeline* (*pipeline)(), char const* quadShaderPath, bool vsync)
{
#if GLEX_COMMON_LOGGING
	printf("Starting renderer.\n");
#endif
	s_frameEndEvent = Event::Get(false);
	s_newCommandEvent = Event::Get(true);
	s_frameEndEvent->Set();
	s_renderingThread.Emplace(RenderProcess, ThreadPriority::Normal);
	s_renderingThread->SetData(vsync);
	s_renderingThread->Resume();
	s_pipeline = pipeline();
	if (quadShaderPath != nullptr)
	{
		// 512 KiB + 48 KiB. If we use separate vertex buffer, vertex buffer can reduce to 224 KiB.
		// CAN'T PASS INTEGER VALUE BECAUSE OF ANOTHER RANDOM INTEL DOG SHIT DRIVER BUG.
		s_quadMesh = ResourceManager::CreateMesh(MakeStaticPointer("f2f2u1f4f3f2f2"), k_quadBufferCount * k_quadVertexStride * 4, k_quadBufferCount * 6 * 4);
		s_quadShader = ResourceManager::CreateShader(quadShaderPath);
		if (s_quadShader == nullptr || (s_quadShader.Wait(), s_quadShader.State() != ResourceState::Ready))
			Abort("Cannot launch renderer. Invalid quad shader.");
	}
}

void Renderer::Shutdown()
{
#if GLEX_COMMON_LOGGING
	printf("Terminating renderer.\n");
#endif
	ResourceManager::Release(s_quadMesh);
	ResourceManager::Release(s_quadShader);
	s_pipeline->~Pipeline();		// Desctruct first, clean it's resources.
	RenderCommand command;
	command.command = RenderCommand::Exit;
	s_commandBuffer.Push(command);
	s_renderingThread->Wait();
	Mem::Free(s_pipeline);			// Only then can we free it's memory.
	Event::Release(s_frameEndEvent);
	Event::Release(s_newCommandEvent);
	s_renderingThread.Destroy();
}

#if GLEX_DEBUG_RENDERING
void Renderer::DrawDebugSphere(glm::vec3 const& origin, float radius, glm::vec3 const& color)
{
	static glm::vec2 s_data[] =
	{
		glm::vec2(glm::cos(glm::radians(0.0f)), glm::sin(glm::radians(0.0f))),
		glm::vec2(glm::cos(glm::radians(15.0f)), glm::sin(glm::radians(15.0f))),
		glm::vec2(glm::cos(glm::radians(30.0f)), glm::sin(glm::radians(30.0f))),
		glm::vec2(glm::cos(glm::radians(45.0f)), glm::sin(glm::radians(45.0f))),
		glm::vec2(glm::cos(glm::radians(60.0f)), glm::sin(glm::radians(60.0f))),
		glm::vec2(glm::cos(glm::radians(75.0f)), glm::sin(glm::radians(75.0f))),
		glm::vec2(glm::cos(glm::radians(90.0f)), glm::sin(glm::radians(90.0f))),
		glm::vec2(glm::cos(glm::radians(105.0f)), glm::sin(glm::radians(105.0f))),
		glm::vec2(glm::cos(glm::radians(120.0f)), glm::sin(glm::radians(120.0f))),
		glm::vec2(glm::cos(glm::radians(135.0f)), glm::sin(glm::radians(135.0f))),
		glm::vec2(glm::cos(glm::radians(150.0f)), glm::sin(glm::radians(150.0f))),
		glm::vec2(glm::cos(glm::radians(165.0f)), glm::sin(glm::radians(165.0f))),
		glm::vec2(glm::cos(glm::radians(180.0f)), glm::sin(glm::radians(180.0f))),
		glm::vec2(glm::cos(glm::radians(195.0f)), glm::sin(glm::radians(195.0f))),
		glm::vec2(glm::cos(glm::radians(210.0f)), glm::sin(glm::radians(210.0f))),
		glm::vec2(glm::cos(glm::radians(225.0f)), glm::sin(glm::radians(225.0f))),
		glm::vec2(glm::cos(glm::radians(240.0f)), glm::sin(glm::radians(240.0f))),
		glm::vec2(glm::cos(glm::radians(255.0f)), glm::sin(glm::radians(255.0f))),
		glm::vec2(glm::cos(glm::radians(270.0f)), glm::sin(glm::radians(270.0f))),
		glm::vec2(glm::cos(glm::radians(285.0f)), glm::sin(glm::radians(285.0f))),
		glm::vec2(glm::cos(glm::radians(300.0f)), glm::sin(glm::radians(300.0f))),
		glm::vec2(glm::cos(glm::radians(315.0f)), glm::sin(glm::radians(315.0f))),
		glm::vec2(glm::cos(glm::radians(330.0f)), glm::sin(glm::radians(330.0f))),
		glm::vec2(glm::cos(glm::radians(345.0f)), glm::sin(glm::radians(345.0f))),
		glm::vec2(glm::cos(glm::radians(360.0f)), glm::sin(glm::radians(360.0f))),
	};
	static glm::vec3 s_poses[std::size(s_data)];
	for (uint32_t i = 0; i < std::size(s_data); i++)
	{
		glm::vec2 entry = s_data[i] * radius;
		s_poses[i] = glm::vec3(entry.x, 0.0f, entry.y) + origin;
	}
	DrawDebugLine(s_poses, s_poses + std::size(s_data), color);
	for (uint32_t i = 0; i < std::size(s_data); i++)
	{
		glm::vec2 entry = s_data[i] * radius;
		s_poses[i] = glm::vec3(entry.x, entry.y, 0.0f) + origin;
	}
	DrawDebugLine(s_poses, s_poses + std::size(s_data), color);
	for (uint32_t i = 0; i < std::size(s_data); i++)
	{
		glm::vec2 entry = s_data[i] * radius;
		s_poses[i] = glm::vec3(0.0f, entry.x, entry.y) + origin;
	}
	DrawDebugLine(s_poses, s_poses + std::size(s_data), color);
}
#endif

/***********************************************
 ************    RENDER COMMANDS    ************
 ***********************************************/
void Renderer::SetViewPort(uint32_t width, uint32_t height)
{
	RenderCommand command;
	command.command = RenderCommand::ViewPort;
	command.param = std::make_pair(width, height);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetFunctionEnabled(RenderCommand::Function function, bool enabled)
{
	RenderCommand command;
	command.command = enabled ? RenderCommand::Enable : RenderCommand::Disable;
	command.param = function;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetBlendingFunction(RenderCommand::BlendingFactor srcColor, RenderCommand::BlendingFactor dstColor, RenderCommand::BlendingFactor srcAlpha, RenderCommand::BlendingFactor dstAlpha)
{
	RenderCommand command;
	command.command = RenderCommand::BlendFunc;
	command.param = std::make_pair(srcColor, dstColor);
	command.in = std::make_pair(srcAlpha, dstAlpha);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::ClearColor(uint32_t i, float r, float g, float b, float a)
{
	RenderCommand command;
	command.command = RenderCommand::ClearColor;
	command.flags = i;
	command.param = glm::vec4(r, g, b, a);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::ClearDepth(float depth)
{
	RenderCommand command;
	command.command = RenderCommand::ClearDepth;
	command.param = depth;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::ColorMask(bool r, bool g, bool b, bool a)
{
	RenderCommand command;
	command.command = RenderCommand::ColorMask;
	command.flags = glm::bvec4(r, g, b, a);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DepthMask(bool d)
{
	RenderCommand command;
	command.command = RenderCommand::DepthMask;
	command.flags = d;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::Draw()
{
	RenderCommand command;
	command.command = RenderCommand::Draw;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::Draw(uint32_t indexCount)
{
	RenderCommand command;
	command.command = RenderCommand::DrawWithIndexCount;
	command.param = indexCount;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DrawLines(uint32_t indexCount)
{
	RenderCommand command;
	command.command = RenderCommand::DrawLinesWithIndexCount;
	command.param = indexCount;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::EndFrame()
{
	SwapBuffers();
	s_frameEndEvent->Wait();
	RenderCommand command;
	command.command = RenderCommand::EndFrame;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
	g_frameAllocator.Swap();
}

void Renderer::SwapBuffers()
{
	RenderCommand command;
	command.command = RenderCommand::SwapBuffers;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetVSync(bool enabled)
{
	RenderCommand command;
	command.command = enabled ? RenderCommand::EnableVSync : RenderCommand::DisableVSync;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

/**************************************************
 ************    INTERNAL RESOURCES    ************
 **************************************************/
void Renderer::CreateBuffer(GlBuffer* buffer, uint32_t size)
{
	RenderCommand command;
	command.command = RenderCommand::CreateDynamicBuffer;
	command.param = size;
	command.out = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::CreateBuffer(GlBuffer* buffer, Pointer<const void> data, uint32_t size)
{
	RenderCommand command;
	command.command = RenderCommand::CreateStaticBuffer;
	command.param = size;
	command.in = data;
	command.out = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyBuffer(GlBuffer* buffer)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyBuffer;
	command.in = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::ReallocateBuffer(GlBuffer* buffer, uint32_t size)
{
	RenderCommand command;
	command.command = RenderCommand::ReallocateBuffer;
	command.flags = 0;
	command.param = size;
	command.in = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::UpdateBuffer(GlBuffer* buffer, uint32_t offset, Pointer<const void> data, uint32_t size)
{
	RenderCommand command;
	command.command = RenderCommand::UpdateBuffer;
	command.param = std::make_pair(offset, size);
	command.in = data;
	command.out = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindBuffer(GlBuffer const* buffer, RenderCommand::BufferTarget target)
{
	RenderCommand command;
	command.command = RenderCommand::BindBuffer;
	command.param = target;
	command.in = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindBufferBase(GlBuffer const* buffer, RenderCommand::BufferTarget target, uint32_t index)
{
	RenderCommand command;
	command.command = RenderCommand::BindBufferBase;
	command.param = std::make_pair(target, index);
	command.in = buffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::CreateFrameBuffer(uint32_t width, uint32_t height, FrameBuffer* out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateFrameBuffer;
	command.param = std::make_pair(width, height);
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyFrameBuffer(FrameBuffer* frameBuffer)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyFrameBuffer;
	command.in = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::ResizeFrameBuffer(FrameBuffer* frameBuffer, uint32_t width, uint32_t height)
{
	RenderCommand command;
	command.command = RenderCommand::ResizeFrameBuffer;
	command.param = std::make_pair(width, height);
	command.in = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddColorAttachment(FrameBuffer* frameBuffer, uint32_t slot, Pointer<const TextureFormat> internalFormat)
{
	RenderCommand command;
	command.command = RenderCommand::AddTextureColorAttachment;
	command.param = slot;
	command.in = internalFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddColorAttachment(FrameBuffer* frameBuffer, uint32_t slot, Pointer<const TextureFormat> internalFormat, uint32_t msaaSamples)
{
	RenderCommand command;
	command.command = RenderCommand::AddMultisampleTextureColorAttachment;
	command.param = std::make_pair(slot, msaaSamples);
	command.in = internalFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddColorAttachment(FrameBuffer* frameBuffer, uint32_t slot, uint32_t rboFormat)
{
	RenderCommand command;
	command.command = RenderCommand::AddRenderBufferColorAttachment;
	command.param = slot;
	command.in = rboFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddColorAttachment(FrameBuffer* frameBuffer, uint32_t slot, uint32_t rboFormat, uint32_t msaaSamples)
{
	RenderCommand command;
	command.command = RenderCommand::AddMultisampleRenderBufferColorAttachment;
	command.param = std::make_pair(slot, msaaSamples);
	command.in = rboFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddDepthAttachment(FrameBuffer* frameBuffer, uint32_t rboFormat)
{
	RenderCommand command;
	command.command = RenderCommand::AddRenderBufferDepthAttachment;
	command.param = rboFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddDepthAttachment(FrameBuffer* frameBuffer, uint32_t rboFormat, uint32_t msaaSamples)
{
	RenderCommand command;
	command.command = RenderCommand::AddMultisampleRenderBufferDepthAttachment;
	command.param = std::make_pair(rboFormat, msaaSamples);
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddDepthAttachment(FrameBuffer* frameBuffer, Pointer<const TextureFormat> internalFormat)
{
	RenderCommand command;
	command.command = RenderCommand::AddTextureDepthAttachment;
	command.in = internalFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::AddDepthAttachment(FrameBuffer* frameBuffer, uint32_t size, Pointer<const TextureFormat> internalFormat)
{
	RenderCommand command;
	command.command = RenderCommand::AddTextureArrayDepthAttachment;
	command.param = size;
	command.in = internalFormat;
	command.out = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindFrameBuffer(FrameBuffer* frameBuffer)
{
	RenderCommand command;
	command.command = RenderCommand::BindFrameBuffer;
	command.in = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindReadFrameBuffer(FrameBuffer* frameBuffer)
{
	RenderCommand command;
	command.command = RenderCommand::BindReadFrameBuffer;
	command.in = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindDrawFrameBuffer(FrameBuffer* frameBuffer)
{
	RenderCommand command;
	command.command = RenderCommand::BindDrawFrameBuffer;
	command.in = frameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::UnbindFrameBuffer()
{
	RenderCommand command;
	command.command = RenderCommand::UnbindFrameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::UnbindReadFrameBuffer()
{
	RenderCommand command;
	command.command = RenderCommand::UnbindReadFrameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::UnbindDrawFrameBuffer()
{
	RenderCommand command;
	command.command = RenderCommand::UnbindDrawFrameBuffer;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BlitFrameBuffer(uint32_t width, uint32_t height)
{
	RenderCommand command;
	command.command = RenderCommand::BlitFrameBuffer;
	command.param = std::make_pair(width, height);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

/***************************************
 ************    SHADERS    ************
 ***************************************/
void Renderer::CreateShader(Pointer<const char> src, RefCountedResource<Shader>* out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateShader;
	command.in = src;
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyShader(RefCountedResource<Shader>* shader)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyShader;
	command.in = shader;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindShader(ResPtr<Shader> shader)
{
	RenderCommand command;
	command.command = RenderCommand::BindShader;
	command.in = shader;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
	RenderCommand command;
	command.command = RenderCommand::DispatchCompute;
	command.param = glm::uvec3(numGroupsX, numGroupsY, numGroupsZ);
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetShaderUniform(ResPtr<Shader> shader, uint32_t location, uint32_t value)
{
	RenderCommand command;
	command.command = RenderCommand::SetShaderUniform1ui;
	command.param = location;
	command.in = value;
	command.out = shader;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

/****************************************
 ************    TEXTURES    ************
 ****************************************/
void Renderer::CreateTexture(Pointer<const uint8_t> data, uint32_t dataFormat, Pointer<const TextureFormat> internalFormat, RefCountedResource<Texture>*out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateTexture;
	command.flags = dataFormat;
	command.param = internalFormat;
	command.in = data;
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::CreateTextureArray(uint32_t size, Pointer<const TextureFormat> internalFormat, RefCountedResource<Texture>* out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateTextureArray;
	command.param = size;
	command.in = internalFormat;
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyTexture(RefCountedResource<Texture>* texture)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyTexture;
	command.in = texture;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindTexture(Texture const* texture, uint32_t slot)
{
	RenderCommand command;
	command.command = RenderCommand::BindTexture;
	command.param = slot;
	command.in = texture;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

/**************************************
 ************    MESHES    ************
 **************************************/
void Renderer::CreateMesh(Pointer<const char> layout, Pointer<const void> vertexData, uint32_t vertexSize, Pointer<const void> indexData, uint32_t indexSize, glm::vec4 const& boundingSphere, RefCountedResource<Mesh>* out)
{
	struct Param
	{
		Pointer<const char> layout;
		Pointer<const void> vertexData, indexData;
		uint32_t vertexSize, indexSize;
		glm::vec4 boundingSphere;
	};

	Param* param = AllocateFrameMemory<Param>();
	param->layout = layout;
	param->vertexData = vertexData;
	param->indexData = indexData;
	param->vertexSize = vertexSize;
	param->indexSize = indexSize;
	param->boundingSphere = boundingSphere;

	RenderCommand command;
	command.command = RenderCommand::CreateMesh;
	command.param = MakeAllocatedPointer(param);
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}


void Renderer::CreateMesh(Pointer<const char> layout, uint32_t vertexSize, uint32_t indexSize, RefCountedResource<Mesh>* out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateEmptyMesh;
	command.param = layout;
	command.in = std::make_pair(vertexSize, indexSize);
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyMesh(RefCountedResource<Mesh>* mesh)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyMesh;
	command.in = mesh;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindMesh(ResPtr<Mesh> mesh)
{
	RenderCommand command;
	command.command = RenderCommand::BindMesh;
	command.in = mesh;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

/*****************************************
 ************    MATERIALS    ************
 *****************************************/
void Renderer::CreateMaterial(ResPtr<Shader> geometryShader, ResPtr<Shader> shadowShader, ResPtr<Shader> colorShader, uint32_t uniformBufferSize, Pointer<const void> uniformBufferData, uint32_t textureCount, Pointer<ResPtr<Texture>> textures, RefCountedResource<Material>* out)
{
	struct Param
	{
		ResPtr<Shader> geometryShader, shadowShader, colorShader;
		Pointer<const void> uniformBufferData;
		Pointer<ResPtr<Texture>> textures;
	};

	Param* param = AllocateFrameMemory<Param>();
	param->geometryShader = geometryShader;
	param->shadowShader = shadowShader;
	param->colorShader = colorShader;
	param->uniformBufferData = uniformBufferData;
	param->textures = textures;
	RenderCommand command;
	command.command = RenderCommand::LoadMaterial;
	command.param = std::make_pair(uniformBufferSize, textureCount);
	command.in = MakeAllocatedPointer(param);
	command.out = out;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::CreateMaterial(ResPtr<Shader> geometryShader, ResPtr<Shader> shadowShader, ResPtr<Shader> colorShader, RefCountedResource<Material>* out)
{
	RenderCommand command;
	command.command = RenderCommand::CreateMaterial;
	command.param = reinterpret_cast<uint64_t>(geometryShader.GetPointer()) | reinterpret_cast<uint64_t>(shadowShader.GetPointer()) << 16 & 0xffff'0000'0000'0000;
	command.in = reinterpret_cast<uint64_t>(colorShader.GetPointer()) | reinterpret_cast<uint64_t>(shadowShader.GetPointer()) << 32 & 0xffff'0000'0000'0000;
	command.out = reinterpret_cast<uint64_t>(out) | reinterpret_cast<uint64_t>(shadowShader.GetPointer()) << 48 & 0xffff'0000'0000'0000;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::DestroyMaterial(RefCountedResource<Material>* material)
{
	RenderCommand command;
	command.command = RenderCommand::DestroyMaterial;
	command.in = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::BindMaterial(ResPtr<Material> material, RenderPass pass)
{
	RenderCommand command;
	command.command = RenderCommand::BindMaterial;
	command.param = pass;
	command.in = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetMaterialTexture(ResPtr<Material> material, uint32_t index, ResPtr<Texture> texture)
{
	RenderCommand command;
	command.command = RenderCommand::SetMaterialTextureByIndex;
	command.flags = index;
	command.param = texture;
	command.in = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetMaterialTexture(ResPtr<Material> material, Pointer<const char> name, ResPtr<Texture> texture)
{
	RenderCommand command;
	command.command = RenderCommand::SetMaterialTextureByName;
	command.param = name;
	command.in = texture;
	command.out = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetMaterialFloat(ResPtr<Material> material, Pointer<const char> name, float value)
{
	RenderCommand command;
	command.command = RenderCommand::SetMaterialFloatByName;
	command.param = name;
	command.in = value;
	command.out = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}

void Renderer::SetMaterialVec3(ResPtr<Material> material, Pointer<const char> name, glm::vec3 const& value)
{
	RenderCommand command;
	command.command = RenderCommand::SetMaterialVec3ByName;
	command.flags = value.b;
	command.param = name;
	command.in = glm::vec2(value.r, value.g);
	command.out = material;
	s_commandBuffer.Push(command);
	s_newCommandEvent->Set();
}