#include "renderer.h"
#include "Context/app.h"
#include <glm/gtc/type_ptr.hpp>
#if GLEX_COMMON_LOGGING || GLEX_REPORT_RESOURCE_ERRORS
#include <stdio.h>
#endif

using namespace glex;
using namespace glex::impl;

void Renderer::RenderProcess(Thread* thread)
{
	static RefCountedResource<Shader>* s_currentShader = nullptr;
	static RefCountedResource<VertexLayout> const* s_currentVertexLayout = nullptr;
	static GlBuffer const* s_currentIndexBuffer = nullptr;

	Window::SetupContext(thread->GetData<bool>());
	RenderCommand command;
	for (;;)
	{
		s_newCommandEvent->Reset();
		if (!s_commandBuffer.Pop(command))
		{
			s_newCommandEvent->Wait();
			s_commandBuffer.Pop(command);
		}
		switch (command.command)
		{
			/***********************************************
			 ************    RENDER COMMANDS    ************
			 ***********************************************/
			case RenderCommand::Exit: return;
			case RenderCommand::ViewPort:
			{
				auto& [width, height] = command.param.AsPair<uint32_t>();
				glViewport(0, 0, width, height);
				break;
			}
			// Break this command into two may eliminate a branch.
			case RenderCommand::Enable:
			{
				uint32_t& function = command.param.As<uint32_t>();
				glEnable(function);
				break;
			}
			case RenderCommand::Disable:
			{
				uint32_t& function = command.param.As<uint32_t>();
				glDisable(function);
				break;
			}
			case RenderCommand::BlendFunc:
			{
				auto& [srcColor, dstColor] = command.param.AsPair<RenderCommand::BlendingFactor>();
				auto& [srcAlpha, dstAlpha] = command.in.AsPair<RenderCommand::BlendingFactor>();
				glBlendFuncSeparate(srcColor, dstColor, srcAlpha, dstAlpha);
				break;
			}
			case RenderCommand::ClearColor:
			{
				glClearBufferfv(GL_COLOR, command.flags.As<uint32_t>(), glm::value_ptr(command.param.As<glm::vec4>()));
				break;
			}
			case RenderCommand::ClearDepth:
			{
				glClearBufferfv(GL_DEPTH, 0, &command.param.As<float>());
				break;
			}
			case RenderCommand::ColorMask:
			{
				glm::bvec4 mask = command.flags.As<glm::bvec4>();
				glColorMask(mask.r, mask.g, mask.b, mask.a);
				break;
			}
			case RenderCommand::DepthMask:
			{
				glDepthMask(command.param.As<bool>());
				break;
			}
			case RenderCommand::Draw:
			{
				if (s_currentIndexBuffer != nullptr)
					glDrawElements(GL_TRIANGLES, s_currentIndexBuffer->Size() >> 2, GL_UNSIGNED_INT, nullptr);
				break;
			}
			case RenderCommand::DrawWithIndexCount:
			{
				if (s_currentIndexBuffer != nullptr)
				{
					uint32_t& indexCount = command.param.As<uint32_t>();
					glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
				}
				break;
			}
			case RenderCommand::DrawLinesWithIndexCount:
			{
				if (s_currentIndexBuffer != nullptr)
				{
					uint32_t& indexCount = command.param.As<uint32_t>();
					glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
				}
				break;
			}
			case RenderCommand::EndFrame:
			{
				// Address may be recycled by resource manager.
				s_currentShader = nullptr;
				s_currentVertexLayout = nullptr;
				s_currentIndexBuffer = nullptr;
				s_frameEndEvent->Set();
				break;
			}
			case RenderCommand::SwapBuffers:
			{
				Window::SwapBuffers();
				break;
			}
			case RenderCommand::EnableVSync:
			{
				Window::SetVSync(true);
				break;
			}
			case RenderCommand::DisableVSync:
			{
				Window::SetVSync(false);
				break;
			}
			/**************************************************
			 ************    INTERNAL RESOURCES    ************
			 **************************************************/
			case RenderCommand::CreateStaticBuffer:
			{
				uint32_t& size = command.param.As<uint32_t>();
				Pointer<const void>& data = command.in.AsPointer<const void>();
				GlBuffer*& buffer = command.out.As<GlBuffer*>();
				new (buffer) GlBuffer(data.GetPointer(), size);
				data.Free();
				break;
			}
			case RenderCommand::CreateDynamicBuffer:
			{
				uint32_t& size = command.param.As<uint32_t>();
				GlBuffer*& buffer = command.out.As<GlBuffer*>();
				new (buffer) GlBuffer(size);
				break;
			}
			case RenderCommand::DestroyBuffer:
			{
				GlBuffer*& buffer = command.in.As<GlBuffer*>();
				buffer->~GlBuffer();
				break;
			}
			case RenderCommand::ReallocateBuffer:
			{
				GlBuffer*& buffer = command.in.As<GlBuffer*>();
				uint32_t& size = command.param.As<uint32_t>();
				buffer->Reallocate(size);
				break;
			}
			case RenderCommand::UpdateBuffer:
			{
				auto& [offset, size] = command.param.AsPair<uint32_t>();
				Pointer<const void>& data = command.in.AsPointer<const void>();
				GlBuffer*& buffer = command.out.As<GlBuffer*>();
				buffer->Set(offset, data.GetPointer(), size);
				data.Free();
				break;
			}
			case RenderCommand::BindBuffer:
			{
				uint32_t& target = command.param.As<uint32_t>();
				GlBuffer*& buffer = command.in.As<GlBuffer*>();
				if (target == GL_ARRAY_BUFFER)
					glBindVertexBuffer(0, buffer->Handle(), 0, (*s_currentVertexLayout)->Stride());
				else
				{
					glBindBuffer(target, buffer->Handle());
					if (target == GL_ELEMENT_ARRAY_BUFFER)
						s_currentIndexBuffer = buffer;
				}
				break;
			}
			case RenderCommand::BindBufferBase:
			{
				auto& [target, index] = command.param.AsPair<uint32_t>();
				GlBuffer*& buffer = command.in.As<GlBuffer*>();
				glBindBufferBase(target, index, buffer->Handle());
				break;
			}
			case RenderCommand::CreateFrameBuffer:
			{
				auto& [width, height] = command.param.AsPair<uint32_t>();
				FrameBuffer*& buffer = command.out.As<FrameBuffer*>();
				new (buffer) FrameBuffer(width, height);
				break;
			}
			case RenderCommand::DestroyFrameBuffer:
			{
				FrameBuffer*& frameBuffer = command.in.As<FrameBuffer*>();
				frameBuffer->~FrameBuffer();
				break;
			}
			case RenderCommand::ResizeFrameBuffer:
			{
				auto& [width, height] = command.param.AsPair<uint32_t>();
				FrameBuffer*& frameBuffer = command.in.As<FrameBuffer*>();
				frameBuffer->Resize(width, height);
				break;
			}
			case RenderCommand::AddTextureColorAttachment:
			{
				uint32_t& slot = command.param.As<uint32_t>();
				Pointer<TextureFormat>& info = command.in.AsPointer<TextureFormat>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddColorAttachment(slot, *info.GetPointer());
				info.Free();
				break;
			}
			case RenderCommand::AddMultisampleTextureColorAttachment:
			{
				auto& [slot, msaaSamples] = command.param.AsPair<uint32_t>();
				Pointer<TextureFormat>& info = command.in.AsPointer<TextureFormat>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddColorAttachment(slot, *info.GetPointer(), msaaSamples);
				info.Free();
				break;
			}
			case RenderCommand::AddRenderBufferColorAttachment:
			{
				uint32_t& slot = command.param.As<uint32_t>();
				uint32_t& format = command.in.As<uint32_t>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddColorAttachment(slot, format);
				break;
			}
			case RenderCommand::AddMultisampleRenderBufferColorAttachment:
			{
				auto& [slot, msaaSamples] = command.param.AsPair<uint32_t>();
				uint32_t& format = command.in.As<uint32_t>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddColorAttachment(slot, format, msaaSamples);
				break;
			}
			case RenderCommand::AddTextureDepthAttachment:
			{
				Pointer<TextureFormat>& info = command.in.AsPointer<TextureFormat>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddDepthAttachment(*info.GetPointer());
				info.Free();
				break;
			}
			case RenderCommand::AddTextureArrayDepthAttachment:
			{
				uint32_t& size = command.param.As<uint32_t>();
				Pointer<TextureFormat>& info = command.in.AsPointer<TextureFormat>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddDepthAttachment(size, *info.GetPointer());
				info.Free();
				break;
			}
			case RenderCommand::AddRenderBufferDepthAttachment:
			{
				uint32_t& format = command.param.As<uint32_t>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddDepthAttachment(format);
				break;
			}
			case RenderCommand::AddMultisampleRenderBufferDepthAttachment:
			{
				auto& [format, msaaSamples] = command.param.AsPair<uint32_t>();
				FrameBuffer*& frameBuffer = command.out.As<FrameBuffer*>();
				frameBuffer->AddDepthAttachment(format, msaaSamples);
				break;
			}
			case RenderCommand::BindFrameBuffer:
			{
				FrameBuffer*& frameBuffer = command.in.As<FrameBuffer*>();
				frameBuffer->Bind();
				break;
			}
			case RenderCommand::BindDrawFrameBuffer:
			{
				FrameBuffer*& frameBuffer = command.in.As<FrameBuffer*>();
				frameBuffer->BindDraw();
				break;
			}
			case RenderCommand::BindReadFrameBuffer:
			{
				FrameBuffer*& frameBuffer = command.in.As<FrameBuffer*>();
				frameBuffer->BindRead();
				break;
			}
			case RenderCommand::UnbindFrameBuffer:
			{
				FrameBuffer::Unbind();
				break;
			}
			case RenderCommand::UnbindReadFrameBuffer:
			{
				FrameBuffer::UnbindRead();
				break;
			}
			case RenderCommand::UnbindDrawFrameBuffer:
			{
				FrameBuffer::UnbindDraw();
				break;
			}
			case RenderCommand::BlitFrameBuffer:
			{
				auto& [width, height] = command.param.AsPair<uint32_t>();
				glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				break;
			}
			/***************************************
			 ************    SHADERS    ************
			 ***************************************/
			case RenderCommand::CreateShader:
			{
				Pointer<const char>& src = command.in.AsPointer<const char>();
				RefCountedResource<Shader>*& shader = command.out.AsResource<Shader>();
#if GLEX_REPORT_RESOURCE_ERRORS
				char const* name = shader->Name().data();
				printf("Compiling shader %s.\n", name[0] == 0 ? name + 1 : name);
#endif
				shader->Emplace(src.GetPointer());
				if (shader->State() != ResourceState::Ready)
				{
					ResourceManager::SetInvalid(shader->Name());
#if GLEX_REPORT_RESOURCE_ERRORS
					printf(GLEX_LOG_RED("Cannot create shader %s.\n%s\n"), name[0] == 0 ? name + 1 : name, Shader::GetErrorMessage().c_str());
#endif
				}
				src.Free();
				break;
			}
			case RenderCommand::DestroyShader:
			{
				RefCountedResource<Shader>*& shader = command.in.AsResource<Shader>();
				shader->Destroy();
				Mem::Delete(shader);
				break;
			}
			case RenderCommand::BindShader:
			{
				ResPtr<Shader>& shader = command.in.AsResPtr<Shader>();
				glUseProgram(shader->Handle());
				s_currentShader = shader.GetPointer();
				break;
			}
			case RenderCommand::DispatchCompute:
			{
				glm::uvec3& dim = command.param.As<glm::uvec3>();
				glDispatchCompute(dim.x, dim.y, dim.z);
				break;
			}
			case RenderCommand::SetShaderUniform1ui:
			{
				uint32_t& location = command.param.As<uint32_t>();
				uint32_t& value = command.in.As<uint32_t>();
				ResPtr<Shader>& shader = command.out.AsResPtr<Shader>();
				if (shader.State() == ResourceState::Ready) GLEX_LIKELY
					shader->SetUniform(location, value);
				break;
			}
			/****************************************
			 ************    TEXTURES    ************
			 ****************************************/
			case RenderCommand::CreateTexture:
			{
				uint32_t& dataFormat = command.flags.As<uint32_t>();
				Pointer<const TextureFormat>& internalFormat = command.param.AsPointer<const TextureFormat>();
				Pointer<const uint8_t>& data = command.in.AsPointer<const uint8_t>();
				RefCountedResource<Texture>*& texture = command.out.AsResource<Texture>();
				texture->Emplace(data.GetPointer(), dataFormat, *internalFormat.GetPointer());
				if (texture->State() != ResourceState::Ready)
				{
					ResourceManager::SetInvalid(texture->Name());
#if GLEX_REPORT_RESOURCE_ERRORS
					printf(GLEX_LOG_RED("Cannot create texture %s.\n"), texture->Name().data());
#endif
				}
				internalFormat.Free();
				data.Free();
				break;
			}
			case RenderCommand::CreateTextureArray:
			{
				uint32_t& size = command.param.As<uint32_t>();
				Pointer<const TextureFormat>& internalFormat = command.in.AsPointer<const TextureFormat>();
				RefCountedResource<Texture>*& texture = command.out.AsResource<Texture>();
				texture->Emplace(size, *internalFormat.GetPointer());
				if (texture->State() != ResourceState::Ready)
				{
#if GLEX_REPORT_RESOURCE_ERRORS
					printf(GLEX_LOG_RED("Cannot create texture array.\n"));
#endif
				}
				internalFormat.Free();
				break;
			}
			case RenderCommand::DestroyTexture:
			{
				RefCountedResource<Texture>*& texture = command.in.AsResource<Texture>();
				texture->Destroy();
				Mem::Delete(texture);
				break;
			}
			case RenderCommand::BindTexture:
			{
				uint32_t& slot = command.param.As<uint32_t>();
				Texture const*& texture = command.in.As<Texture const*>();
				glBindTextureUnit(slot, texture->Handle());
				break;
			}
			/**************************************
			 ************    MESHES    ************
			 **************************************/
			case RenderCommand::CreateMesh:
			{
				struct Param
				{
					Pointer<const char> layout;
					Pointer<const void> vertexData, indexData;
					uint32_t vertexSize, indexSize;
					glm::vec4 boundingSphere;
				};

				Pointer<Param>& param = command.param.AsPointer<Param>();
				RefCountedResource<Mesh>*& mesh = command.out.AsResource<Mesh>();
				Param* realParam = param.GetPointer();
				ResPtr<VertexLayout> layout = ResourceManager::RequestVertexLayout(realParam->layout.GetPointer());
				if (layout == nullptr)
				{
					mesh->Invalidate();
					ResourceManager::SetInvalid(mesh->Name());
#if GLEX_REPORT_RESOURCE_ERRORS
					printf(GLEX_LOG_RED("Cannot create mesh %s, because its layout %s is invalid.\n"), mesh->Name().data(), realParam->layout.GetPointer());
#endif
				}
				else
				{
					mesh->Create(layout, realParam->vertexData.GetPointer(), realParam->vertexSize, realParam->indexData.GetPointer(), realParam->indexSize, realParam->boundingSphere);
					if (mesh->State() != ResourceState::Ready)	// Should always be OK.
					{
						ResourceManager::SetInvalid(mesh->Name());
#if GLEX_REPORT_RESOURCE_ERRORS
						printf(GLEX_LOG_RED("Cannot create mesh %s.\n"), mesh->Name().data());
#endif
					}
				}
				realParam->layout.Free();
				realParam->vertexData.Free();
				realParam->indexData.Free();
				param.Free();
				break;
			}
			case RenderCommand::CreateEmptyMesh:
			{
				Pointer<const char> layout = command.param.AsPointer<const char>();
				auto& [vertexSize, indexSize] = command.in.AsPair<uint32_t>();
				RefCountedResource<Mesh>*& mesh = command.out.AsResource<Mesh>();
				ResPtr<VertexLayout> vao = ResourceManager::RequestVertexLayout(layout.GetPointer());
				if (vao == nullptr)
				{
					mesh->Invalidate();
#if GLEX_REPORT_RESOURCE_ERRORS
					printf(GLEX_LOG_RED("Cannot create empty mesh because its layout %s is invalid.\n"), layout.GetPointer());
#endif
				}
				else
				{
					mesh->Create(vao, vertexSize, indexSize);
#if GLEX_REPORT_RESOURCE_ERRORS
					if (mesh->State() != ResourceState::Ready)
						printf(GLEX_LOG_RED("Cannot create empty mesh.\n"));
#endif
				}
				layout.Free();
				break;
			}
			case RenderCommand::DestroyMesh:
			{
				RefCountedResource<Mesh>*& mesh = command.in.AsResource<Mesh>();
				mesh->Destroy();
				Mem::Delete(mesh);
				break;
			}
			case RenderCommand::BindMesh:
			{
				ResPtr<Mesh>& mesh = command.in.AsResPtr<Mesh>();
				if (s_currentShader == nullptr || mesh.State() != ResourceState::Ready)
				{
					s_currentIndexBuffer = nullptr;	// To indicate mesh is invalid. Do not draw.
					break;
				}
				ResPtr<VertexLayout> layout = mesh->GetVertexLayout();
				if (layout.GetPointer() != s_currentVertexLayout)
				{
					glBindVertexArray(layout->Handle());
					s_currentVertexLayout = layout.GetPointer();
				}
				glBindVertexBuffer(0, mesh->GetVertexBuffer()->Handle(), 0, (*s_currentVertexLayout)->Stride());
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBuffer()->Handle());
				s_currentIndexBuffer = mesh->GetIndexBuffer();
				break;
			}
			/*****************************************
			 ************    MATERIALS    ************
			 *****************************************/
			case RenderCommand::LoadMaterial:
			{
				struct Param
				{
					ResPtr<Shader> geometryShader, shadowShader, colorShader;
					Pointer<const void> uniformBufferData;
					Pointer<ResPtr<Texture>> textures;
				};

				auto& [uniformBufferSize, textureCount] = command.param.AsPair<uint32_t>();
				Pointer<Param>& param = command.in.AsPointer<Param>();
				RefCountedResource<Material>*& material = command.out.AsResource<Material>();
				Param* realParam = param.GetPointer();
				auto textures = realParam->textures.GetPointer();

				if (realParam->geometryShader == nullptr || realParam->geometryShader.State() != ResourceState::Ready || realParam->shadowShader != nullptr && realParam->shadowShader.State() != ResourceState::Ready || realParam->colorShader.State() != ResourceState::Ready)
					goto mat_invalid;
				if (uniformBufferSize != realParam->colorShader->UniformBufferSize() || textureCount > realParam->colorShader->TextureCount())
					goto mat_invalid;
				for (uint32_t i = 0; i < textureCount; i++)
				{
					if (textures[i].State() != ResourceState::Ready)
						goto mat_invalid;
				}

				material->Emplace(realParam->geometryShader, realParam->shadowShader, realParam->colorShader, realParam->uniformBufferData.GetPointer(), uniformBufferSize, textureCount, false);
				if (material->State() != ResourceState::Ready)
					goto mat_invalid;
				for (uint32_t i = 0; i < textureCount; i++)
					(*material)->SetTexture(i, textures[i]);
				realParam->uniformBufferData.Free();
				realParam->textures.Free();
				param.Free();
				break;
			mat_invalid:
				ResourceManager::SetInvalid(material->Name());
				realParam->uniformBufferData.Free();
				realParam->textures.Free();
				param.Free();
				break;
			}
			case RenderCommand::CreateMaterial:
			{
				ResPtr<Shader> geometryShader = reinterpret_cast<RefCountedResource<Shader>*>(command.param.As<uint64_t>() & 0x0000'ffff'ffff'ffff);
				ResPtr<Shader> shadowShader = reinterpret_cast<RefCountedResource<Shader>*>((command.param.As<uint64_t>() & 0xffff'0000'0000'0000) >> 16 | (command.in.As<uint64_t>() & 0xffff'0000'0000'0000) >> 32 | (command.out.As<uint64_t>() & 0xffff'0000'0000'0000) >> 48);
				ResPtr<Shader> colorShader = reinterpret_cast<RefCountedResource<Shader>*>(command.in.As<uint64_t>() & 0x0000'ffff'ffff'ffff);
				RefCountedResource<Material>* material = reinterpret_cast<RefCountedResource<Material>*>(command.out.As<uint64_t>() & 0x0000'ffff'ffff'ffff);
				if (geometryShader == nullptr || geometryShader.State() != ResourceState::Ready || shadowShader != nullptr && shadowShader.State() != ResourceState::Ready || colorShader.State() != ResourceState::Ready)
					material->Invalidate();
				else
					material->Emplace(geometryShader, shadowShader, colorShader, nullptr, colorShader->UniformBufferSize(), colorShader->TextureCount(), true);
				break;
			}
			case RenderCommand::DestroyMaterial:
			{
				RefCountedResource<Material>*& material = command.in.AsResource<Material>();
				material->Destroy();	// Material cannot be properly destroyed if it hasn't even been properly constructed.
				Mem::Delete(material);
				break;
			}
			case RenderCommand::BindMaterial:
			{
				RenderPass pass = command.param.As<RenderPass>();
				ResPtr<Material>& material = command.in.AsResPtr<Material>();
				if (material.State() != ResourceState::Ready)
				{
					s_currentShader = nullptr;	// Indicate that material is invalid, so you cannot draw.
					break;
				}
				ResPtr<Shader> shader = pass == RenderPass::Geometry ? material->GetGeometryShader() : pass == RenderPass::Color ? material->GetColorShader() : material->GetShadowShader();
				if (shader.GetPointer() != s_currentShader)
				{
					if (shader != nullptr)
						glUseProgram(shader->Handle());
					s_currentShader = shader.GetPointer();
				}
				if (pass == RenderPass::Color)
				{
					GlBuffer const* uniformBuffer = material->GetBuffer();
					if (*uniformBuffer != nullptr)
						glBindBufferBase(GL_UNIFORM_BUFFER, 3, uniformBuffer->Handle());
					uint32_t numTextures = material->GetTextureCount();
					for (uint32_t i = 0; i < numTextures; i++)
					{
						ResPtr<Texture> texture = material->GetTexture(i);
						if (texture != nullptr)
							glBindTextureUnit(i, texture->Handle());
					}
				}
				break;
			}
			case RenderCommand::SetMaterialTextureByIndex:
			{
				uint32_t& index = command.flags.As<uint32_t>();
				ResPtr<Texture>& texture = command.param.AsResPtr<Texture>();
				ResPtr<Material>& material = command.in.AsResPtr<Material>();
				ResPtr<Shader> shader = material->GetColorShader();
				if (material.State() != ResourceState::Ready)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture for material %s because material is invalid.\n"), material.Name().data());
#endif
					break;
				}
				if (texture.State() != ResourceState::Ready)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture %s for material because texture is invalid.\n"), texture.Name().data());
#endif
					break;
				}
				if (index >= shader->TextureCount())
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture %d for material %s because material doesn't have such property.\n"), index, material.Name().data());
#endif
					break;
				}
				material->SetTexture(index, texture);
				break;
			}
			case RenderCommand::SetMaterialTextureByName:
			{
				Pointer<const char>& name = command.param.AsPointer<const char>();
				ResPtr<Texture>& texture = command.in.AsResPtr<Texture>();
				ResPtr<Material>& material = command.out.AsResPtr<Material>();
				ResPtr<Shader> shader = material->GetColorShader();
				ShaderProperty const* property;
				if (material.State() != ResourceState::Ready)	// Ignore invalid reosurces.
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture for material %s because material is invalid.\n"), material.Name().data());
#endif
					goto mt_invalid;
				}
				if (texture.State() != ResourceState::Ready)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture %s for material because texture is invalid.\n"), texture.Name().data());
#endif
					goto mt_invalid;
				}
				property = shader->GetProperty(name.GetPointer());
				if (property == nullptr || property->type != ShaderProperty::Texture)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set texture %s for material %s because material doesn't have such property.\n"), name.GetPointer(), material.Name().data());
#endif
					goto mt_invalid;
				}
				material->SetTexture(property->offset, texture);
			mt_invalid:
				name.Free();
				break;
			}
			case RenderCommand::SetMaterialFloatByName:
			{
				Pointer<const char>& name = command.param.AsPointer<const char>();
				float& value = command.in.As<float>();
				ResPtr<Material>& material = command.out.AsResPtr<Material>();
				ResPtr<Shader> shader = material->GetColorShader();
				ShaderProperty const* property;
				if (material.State() != ResourceState::Ready)	// Ignore invalid reosurces.
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set float for material %s because material is invalid.\n"), material.Name().data());
#endif
					goto mf_invalid;
				}
				property = shader->GetProperty(name.GetPointer());
				if (property == nullptr || property->type != ShaderProperty::Float)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set float %s for material %s because material doesn't have such property.\n"), name.GetPointer(), material.Name().data());
#endif
					goto mf_invalid;
				}
				material->Set(property->offset, &value, sizeof(float));
			mf_invalid:
				name.Free();
				break;
			}
			case RenderCommand::SetMaterialVec3ByName:
			{
				float& b = command.flags.As<float>();
				Pointer<const char>& name = command.param.AsPointer<const char>();
				glm::vec2& rg = command.in.As<glm::vec2>();
				ResPtr<Material>& material = command.out.AsResPtr<Material>();
				ResPtr<Shader> shader = material->GetColorShader();
				ShaderProperty const* property;
				glm::vec3 value(rg, b);
				if (material.State() != ResourceState::Ready)	// Ignore invalid reosurces.
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set vec3 for material %s because material is invalid.\n"), material.Name().data());
#endif
					goto mv3_invalid;
				}
				property = shader->GetProperty(name.GetPointer());
				if (property == nullptr || property->type != ShaderProperty::Vec3)
				{
#if GLEX_REPORT_GL_ERRORS
					printf(GLEX_LOG_YELLOW("Cannot set vec3 %s for material %s because material doesn't have such property.\n"), name.GetPointer(), material.Name().data());
#endif
					goto mv3_invalid;
				}
				material->Set(property->offset, glm::value_ptr(value), sizeof(glm::vec3));
			mv3_invalid:
				name.Free();
				break;
			}
			default: break;
		}
	}
}
