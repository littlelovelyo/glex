#pragma once
#include "Core/GL/context.h"
#include "Core/Platform/window.h"
#include "Core/Platform/input.h"
#include "Core/Platform/time.h"
#include "Engine/Renderer/image.h"
#include "Core/Memory/smart_ptr.h"
#include "Engine/Scripting/type.h"
#include "Engine/Renderer/render_pass.h"
#include "Engine/ECS/mesh.h"
#include "Engine/ECS/transform.h"

namespace glex::py
{
	inline uint32_t RenderWidth()
	{
		return gl::Context::Width();
	}

	inline uint32_t RenderHeight()
	{
		return gl::Context::Height();
	}

	inline uint32_t WindowWidth()
	{
		return Window::Width();
	}

	inline uint32_t WindowHeight()
	{
		return Window::Height();
	}

	inline float MouseDeltaX()
	{
		return Input::MouseDeltaX();
	}

	inline float MouseDeltaY()
	{
		return Input::MouseDeltaY();
	}

	inline float MouseX()
	{
		return Input::MouseX();
	}

	inline float MouseY()
	{
		return Input::MouseY();
	}

	inline double Time()
	{
		return Time::Current();
	}

	inline float DeltaTime()
	{
		return Time::DeltaTime();
	}

	inline bool Pressed(uint8_t key)
	{
		return Input::HasPressed(key);
	}

	inline bool Pressing(uint8_t key)
	{
		return Input::IsPressing(key);
	}

	inline bool Released(uint8_t key)
	{
		return Input::HasReleased(key);
	}

	struct Image
	{
		SharedPtr<glex::Image> m_image;

		PyRetVal<void> Create(gl::ImageFormat format, gl::ImageUsage usages, glm::uvec3 size, uint32_t samples, bool usedAsCube);
		void Destroy();
		PyRetVal<void> Resize(uint32_t width, uint32_t height) { return m_image->Resize({ width, height }) ? PyRetVal<void>(PyStatus::Success) : PyRetVal<void>(PyStatus::RaiseException); }
	};

	struct ImageView
	{
		SharedPtr<glex::ImageView> m_imageView;

		PyRetVal<void> Create(Type<Image>* image, uint32_t layerIndex, uint32_t numLayers, gl::ImageType type, gl::ImageAspect aspect);
		void Destroy();
		PyRetVal<void> Recreate() { return m_imageView->Recreate() ? PyRetVal<void>(PyStatus::Success) : PyRetVal<void>(PyStatus::RaiseException); }
	};

	struct RenderPassBuilder
	{
		glex::RenderPass::Builder m_builder;

		void PushSubpass() { m_builder.PushSubpass(); }
		void Read(Type<ImageView>* attachment) { m_builder.Read((*attachment)->m_imageView); }
		void Write(Type<ImageView>* attachment) { m_builder.Write((*attachment)->m_imageView); }
		void Clear(Type<ImageView>* attachment) { m_builder.Clear((*attachment)->m_imageView); }
		void Output(Type<ImageView>* attachment) { m_builder.Output((*attachment)->m_imageView); }
	};

	struct RenderList
	{
		Vector<std::pair<MeshRenderer&, Transform&>> m_meshList;
	};

	struct RenderPass
	{
		Optional<glex::RenderPass> m_renderPass;

		void Create() { m_renderPass.Emplace(); }
		void Destroy() { m_renderPass.Destroy(); }
		Type<RenderPassBuilder>* BeginRenderPassDefinition() { return Type<RenderPassBuilder>::New(); }
		PyRetVal<void> EndRenderPassDefinition(Type<RenderPassBuilder>* builder) { return m_renderPass->EndRenderPassDefinition((*builder)->m_builder) ? PyRetVal<void>(PyStatus::Success) : PyRetVal<void>(PyStatus::RaiseException); }
		PyRetVal<void> BeginRenderPass(PyObject* clearValueList);
		void NextSubpass() { m_renderPass->NextSubpass(); }
		void EndRenderPass() { m_renderPass->EndRenderPass(); }
		void RenderMeshList(Type<RenderList>* list, uint32_t materialDomain);
		PyRetVal<void> Recreate() { return m_renderPass->Recreate() ? PyRetVal<void>(PyStatus::Success) : PyRetVal<void>(PyStatus::RaiseException); }
	};

	struct MaterialDomainDefinition
	{
		Type<RenderPass>* m_renderPass;
		uint32_t m_subpass;
		gl::MetaMaterialInfo m_metaMaterial;
		
		void Create(PyKeywordParameters kwds, Type<RenderPass>* renderPass, uint32_t subpass);
	};
}