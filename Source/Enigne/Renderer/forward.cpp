#include "forward.h"
#include "renderer.h"
#include "Context/app.h"
#include "Context/time.h"
#include "Context/input.h"
#include "Component/skinned.h"
#include "Utils/minmax.h"
#include <EASTL/heap.h>
#if GLEX_COMMON_LOGGING
#include <stdio.h>
#endif

using namespace glex;
using namespace glex::impl;

ForwardRenderer::ForwardRenderer(uint32_t tileSizeX, uint32_t tileSizeY, uint32_t maxLightsInScene, uint32_t maxLightsInTile, uint32_t directionalShadowResolution, uint32_t maxDirectionalShadows, char const* lightShaderPath, char const* debugLineShaderPath)
{
#if GLEX_COMMON_LOGGING
	printf("Creating forward renderer: tile size(%d, %d); max lights in scene(%d); max lights in tile(%d); directional shadow resolution(%d); max directional shadows(%d).\n", tileSizeX, tileSizeY, maxLightsInScene, maxLightsInTile, directionalShadowResolution, maxDirectionalShadows);
#endif
	uint32_t width = Window::Width(), height = Window::Height();
	m_directionalShadowResolution = directionalShadowResolution;
	m_maxDirectionalShadows = maxDirectionalShadows;
	m_horizontalTiles = (width + tileSizeX - 1) / tileSizeX;
	m_verticalTiles = (height + tileSizeY - 1) / tileSizeY;
	m_tileSizeX = tileSizeX;
	m_tileSizeY = tileSizeY;
	m_maxLightCount = maxLightsInScene;
	m_maxLightsInTile = maxLightsInTile;

	TextureFormat* depthFormat = static_cast<TextureFormat*>(g_frameAllocator.Allocate(sizeof(TextureFormat), alignof(TextureFormat)));
	depthFormat->wrapS = Texture::Clamp;
	depthFormat->wrapT = Texture::Clamp;
	depthFormat->filterMin = Texture::Nearest;
	depthFormat->filterMag = Texture::Nearest;
	depthFormat->format = Texture::Depth;
	depthFormat->generateMipmap = false;
	Renderer::CreateFrameBuffer(width, height, &m_depthMap);
	Renderer::AddDepthAttachment(&m_depthMap, MakeStaticPointer(depthFormat));

	Renderer::CreateFrameBuffer(width, height, &m_hdr);
	Renderer::AddColorAttachment(&m_hdr, 0, RenderBuffer::RGB16F, 4);
	Renderer::AddDepthAttachment(&m_hdr, RenderBuffer::Depth, 4);

	if (maxDirectionalShadows != 0)
	{
		TextureFormat* directionalShadow = static_cast<TextureFormat*>(g_frameAllocator.Allocate(sizeof(TextureFormat), alignof(TextureFormat)));
		directionalShadow->wrapS = Texture::Clamp;
		directionalShadow->wrapT = Texture::Clamp;
		directionalShadow->filterMin = Texture::Linear;
		directionalShadow->filterMag = Texture::Linear;
		directionalShadow->width = directionalShadowResolution;
		directionalShadow->height = directionalShadowResolution;
		directionalShadow->format = Texture::Depth;
		Renderer::CreateFrameBuffer(directionalShadowResolution, directionalShadowResolution, &m_directionalShadowMap);
		Renderer::AddDepthAttachment(&m_directionalShadowMap, maxDirectionalShadows, MakeStaticPointer(directionalShadow));
		Renderer::CreateBuffer(&m_directionalShadowData, sizeof(DirectionalShadowData) * maxDirectionalShadows);
		Renderer::BindBufferBase(&m_directionalShadowData, RenderCommand::UniformBuffer, 2);
		Renderer::BindTexture(m_directionalShadowMap->GetDepthTexture(), 31);
	}

	Renderer::CreateBuffer(&m_globalData, sizeof(GlobalDataHeader) + sizeof(GlobalDataRest));
	Renderer::CreateBuffer(&m_boneData, sizeof(glm::mat4) * k_maxBoneCount);
	Renderer::CreateBuffer(&m_lightData, sizeof(LightDataHeader) + sizeof(LightData) * maxLightsInScene);
	Renderer::CreateBuffer(&m_tileData, MakeStaticPointer<const void>(nullptr), 4 * maxLightsInTile * m_horizontalTiles * m_verticalTiles); // Specify nullptr to use GL_STATIC_DRAW.
	/*static float quadVertices[] =
	{
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	static uint32_t quadIndices[] =
	{
		0, 1, 2, 0, 2, 3
	};
	m_screenQuadMesh = ResourceManager::CreateMesh(MakeStaticPointer("f2f2"), MakeStaticPointer(quadVertices), sizeof(quadVertices), MakeStaticPointer(quadIndices), sizeof(quadIndices), glm::vec4(0.0f, 0.0f, 0.0f, INFINITY));*/
	m_lightCullingShader = ResourceManager::CreateShader(lightShaderPath);
	if (m_lightCullingShader == nullptr || (m_lightCullingShader.Wait(), m_lightCullingShader.State() != ResourceState::Ready))
		Abort("Cannot launch forward renderer. Invalid light culling shader.");

#if GLEX_DEBUG_RENDERING
	s_lineShader = ResourceManager::CreateShader(debugLineShaderPath);
	s_debugLines = ResourceManager::CreateMesh(MakeStaticPointer("f3f3"), k_maxDebugLineCount * 48, k_maxDebugLineCount * 8);
	if (s_lineShader == nullptr || (s_lineShader.Wait(), s_lineShader.State() != ResourceState::Ready))
		Abort("Cannot launch forward renderer. Invalid debug line shader.");
#endif

	Renderer::BindBufferBase(&m_globalData, RenderCommand::UniformBuffer, 0);
	Renderer::BindBufferBase(&m_boneData, RenderCommand::UniformBuffer, 1);
	Renderer::BindBufferBase(&m_lightData, RenderCommand::ShaderStorageBuffer, 0);
	Renderer::BindBufferBase(&m_tileData, RenderCommand::ShaderStorageBuffer, 1);
	Renderer::BindTexture(m_depthMap->GetDepthTexture(), 29);
	Renderer::SetBlendingFunction(RenderCommand::SourceAlpha, RenderCommand::OneMinusSourceAlpha, RenderCommand::Zero, RenderCommand::One);
}

ForwardRenderer::~ForwardRenderer()
{
#if GLEX_COMMON_LOGGING
	printf("Destroying forward renderer.\n");
#endif
#if GLEX_DEBUG_RENDERING
	ResourceManager::Release(s_lineShader);
	ResourceManager::Release(s_debugLines);
#endif
	ResourceManager::Release(m_lightCullingShader);
	// ResourceManager::Release(m_screenQuadMesh);
	Renderer::DestroyBuffer(&m_tileData);
	Renderer::DestroyBuffer(&m_lightData);
	Renderer::DestroyBuffer(&m_boneData);
	Renderer::DestroyBuffer(&m_globalData);
	Renderer::DestroyBuffer(&m_directionalShadowData);
	Renderer::DestroyFrameBuffer(&m_hdr);
	Renderer::DestroyFrameBuffer(&m_depthMap);
	Renderer::DestroyFrameBuffer(&m_directionalShadowMap);
}

void ForwardRenderer::OnResize(uint32_t width, uint32_t height)
{
	m_horizontalTiles = (width + m_tileSizeX - 1) / m_tileSizeX;
	m_verticalTiles = (height + m_tileSizeY - 1) / m_tileSizeY;
	Renderer::ResizeFrameBuffer(&m_depthMap, width, height);
	Renderer::ResizeFrameBuffer(&m_hdr, width, height);
	Renderer::ReallocateBuffer(&m_tileData, 4 * m_maxLightsInTile * m_horizontalTiles * m_verticalTiles);
}

void ForwardRenderer::DrawShadowMap(RefCountedResource<Material>*& currentMaterial, FrustumCuller const& frustum, Scene& scene)
{
	scene.ForEach<MeshRenderer, Transform>([&](uint32_t id, MeshRenderer& mr, Transform& tr)
	{
		ResPtr<Material> material = mr.Material();
		ResPtr<Mesh> mesh = mr.Mesh();
		if (material == nullptr || mesh == nullptr || !mr.CastShadow())
			return;
		glm::vec4 boundingSphere = mr.GetBoundingSphere();
		glm::vec3 const& scale = tr.GetGlobalScale();
		boundingSphere *= glm::vec4(scale, Max(scale.x, scale.y, scale.z));
		lval_cast<glm::vec3>(boundingSphere) += tr.GetGlobalPosition();
		if (!frustum.IsInside(boundingSphere))
			return;
		if (material.GetPointer() != currentMaterial)
		{
			Renderer::BindMaterial(material, RenderPass::Shadow);
			currentMaterial = material.GetPointer();
		}
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer<true>(&tr.GetModelMat()), sizeof(glm::mat4));
		Renderer::BindMesh(mesh);
		Renderer::Draw();
	});
	scene.ForEach<SkinnedMeshRenderer, Transform>([&](uint32_t id, SkinnedMeshRenderer& smr, Transform& tr)
	{
		ResPtr<Material> material = smr.Material();
		ResPtr<Mesh> mesh = smr.Mesh();
		if (material == nullptr || mesh == nullptr || !smr.CastShadow())
			return;
		eastl::vector<glm::mat4, Allocator> const& bones = smr.Pose();
		uint32_t size = bones.size() * sizeof(glm::mat4);
		Renderer::UpdateBuffer(&m_boneData, 0, CopyPointer<true>(bones.data(), size), size);
		if (material.GetPointer() != currentMaterial)
		{
			Renderer::BindMaterial(material, RenderPass::Shadow);
			currentMaterial = material.GetPointer();
		}
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer<true>(&tr.GetModelMat()), sizeof(glm::mat4));
		Renderer::BindMesh(mesh);
		Renderer::Draw();
	});
}

void ForwardRenderer::DrawObject(RefCountedResource<Material>*& currentMaterial, Transform& transform, MeshRenderer& meshRenderer)
{
	ResPtr<Material> material = meshRenderer.Material();
	if (material.GetPointer() != currentMaterial)
	{
		Renderer::BindMaterial(material, RenderPass::Color);
		currentMaterial = material.GetPointer();
	}
	glm::mat4 const& modelMat = transform.GetModelMat();
	glm::mat4 normalMat = glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMat))));
	Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer<true>(&modelMat), sizeof(glm::mat4));
	Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader) + sizeof(glm::mat4), CopyPointer<true>(&normalMat), sizeof(glm::mat4));
	ResPtr<Mesh> mesh = meshRenderer.Mesh();
	Renderer::BindMesh(mesh);
	Renderer::Draw();
}

#if GLEX_DEBUG_RENDERING
void ForwardRenderer::DrawDebugLines()
{
	if (s_debugLineIndexCount == 0)
		return;
	Renderer::BindShader(s_lineShader);
	Renderer::BindMesh(s_debugLines);
	Renderer::DrawLines(s_debugLineIndexCount);
	s_debugLineVertexCount = 0;
	s_debugLineIndexCount = 0;
}

void ForwardRenderer::DrawDebugLine(glm::vec3 const* begin, glm::vec3 const* end, glm::vec3 const& color)
{
	for (glm::vec3 const* p = begin; s_debugLineIndexCount < k_maxDebugLineCount * 2 && p != end; p++)
	{
		auto v = std::pair(*p, color);
		Renderer::UpdateBuffer(s_debugLines->GetVertexBuffer(), s_debugLineVertexCount * 24, CopyPointer(&v), 24);
		if (p != begin)
		{
			auto i = std::pair(s_debugLineVertexCount - 1, s_debugLineVertexCount);
			Renderer::UpdateBuffer(s_debugLines->GetIndexBuffer(), s_debugLineIndexCount * 4, CopyPointer(&i), 8);
			s_debugLineIndexCount += 2;
		}
		s_debugLineVertexCount++;
	}
}
#endif

void ForwardRenderer::Render(Scene& scene)
{
	GLEX_LASSERT(scene.GetCamera() != Scene::k_null) {}

	// 0. Update global data.
	auto [cam, camtrans] = scene.GetComponent<Camera, Transform>(scene.GetCamera());
	GlobalDataHeader* globalData = static_cast<GlobalDataHeader*>(g_frameAllocator.Allocate(sizeof(GlobalDataHeader), alignof(GlobalDataHeader)));
	globalData->viewMat = cam.ViewMat(camtrans);
	globalData->projMat = cam.ProjMat();
	globalData->viewProjMat = globalData->projMat * globalData->viewMat;
	globalData->screenSize = { Window::Width(), Window::Height() };
	globalData->time = Time::Current();
	Renderer::UpdateBuffer(&m_globalData, 0, MakeStaticPointer(globalData), sizeof(GlobalDataHeader) - 4);
	FrustumCuller frustum(globalData->viewProjMat);
	ShadowFrustum shadowFrustum(globalData->projMat, globalData->viewProjMat);

	// 1. Depth prepass. May use LODs to reduce triangle count.
	Renderer::BindFrameBuffer(&m_depthMap);
	Renderer::ClearDepth(1.0f);
	Renderer::SetFunctionEnabled(RenderCommand::FaceCulling, true);
	Renderer::SetFunctionEnabled(RenderCommand::DepthTest, true);
	Renderer::SetFunctionEnabled(RenderCommand::Blending, false);
	Renderer::SetViewPort(Window::Width(), Window::Height());
	RefCountedResource<Material>* currentMaterial;
	scene.ForEach<MeshRenderer, Transform>([&](uint32_t id, MeshRenderer& mr, Transform& tr)
	{
		ResPtr<Material> material = mr.Material();
		ResPtr<Mesh> mesh = mr.Mesh();
		if (material == nullptr || mesh == nullptr)
			return;
		glm::vec4 boundingSphere = mr.GetBoundingSphere();
		glm::vec3 const& scale = tr.GetGlobalScale();
		boundingSphere *= glm::vec4(scale, Max(scale.x, scale.y, scale.z));
		lval_cast<glm::vec3>(boundingSphere) += tr.GetGlobalPosition();
		if (!frustum.IsInside(boundingSphere))
			return;
		if (mr.Order() == RenderOrder::Transparent)
		{
			glm::vec3 view = camtrans.GetGlobalPosition() - tr.GetGlobalPosition();
			m_transparentQueue.emplace_back(&tr, &mr, glm::dot(view, view));
			eastl::push_heap(m_transparentQueue.begin(), m_transparentQueue.end(), [](TransparentItem const& lhs, TransparentItem const& rhs)
			{
				return lhs.distance < rhs.distance;
			});
			return;
		}
		m_opaqueQueue.emplace_back(&tr, &mr);
		if (material.GetPointer() != currentMaterial)
		{
			Renderer::BindMaterial(material, RenderPass::Geometry);
			currentMaterial = material.GetPointer();
		}
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer<true>(&tr.GetModelMat()), sizeof(glm::mat4));
		Renderer::BindMesh(mesh);
		Renderer::Draw();
	});
	scene.ForEach<SkinnedMeshRenderer, Transform>([&](uint32_t id, SkinnedMeshRenderer& smr, Transform& tr)
	{
		ResPtr<Material> material = smr.Material();
		ResPtr<Mesh> mesh = smr.Mesh();
		if (material == nullptr || mesh == nullptr)
			return;
		smr.Update(id, tr);
		eastl::vector<glm::mat4, Allocator> const& bones = smr.Pose();
		uint32_t size = bones.size() * sizeof(glm::mat4);
		Renderer::UpdateBuffer(&m_boneData, 0, CopyPointer<true>(bones.data(), size), size);
		if (material.GetPointer() != currentMaterial)
		{
			Renderer::BindMaterial(material, RenderPass::Geometry);
			currentMaterial = material.GetPointer();
		}
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer<true>(&tr.GetModelMat()), sizeof(glm::mat4));
		Renderer::BindMesh(mesh);
		Renderer::Draw();
	});

	// 2. Collect all lights in the scene.
	void* lightData = g_frameAllocator.Allocate(sizeof(LightDataHeader) + sizeof(LightData) * m_maxLightCount, alignof(LightDataHeader));
	LightDataHeader* lightDataHeader = static_cast<LightDataHeader*>(lightData);
	LightData* lightDataArray = reinterpret_cast<LightData*>(lightDataHeader + 1);
	uint32_t lightIndex = 0;
	uint32_t directionalShadowCount = 0;
	scene.ForEach<Light, Transform>([&](uint32_t id, Light& light, Transform& trans)
	{
		if (lightIndex >= m_maxLightCount)
			return;
		LightData& lightData = lightDataArray[lightIndex];
		lightData.color = light.Color();
		lightData.radius = light.Radius();
		lightData.direction = light.GetType() == Light::Directional ? -trans.Forward() : trans.GetGlobalPosition();
		lightData.type = light.GetType();
		lightData.shadowIndex = -1;
		if (light.ShadowCascadeCount() != 0)
		{
			if (light.GetType() == Light::Directional)
			{
				m_directionalShadowQueue.emplace_back(&light, lightData.direction, glm::dot(light.Color(), light.Color()), lightIndex, light.ShadowCascadeCount());
				eastl::push_heap(m_directionalShadowQueue.begin(), m_directionalShadowQueue.end(), DirectionalLight::Compare);
				directionalShadowCount += light.ShadowCascadeCount();
				while (directionalShadowCount > m_maxDirectionalShadows)
				{
					directionalShadowCount -= m_directionalShadowQueue.back().cascadeCount;
					m_directionalShadowQueue.pop_back();
				}
			}
		}
		lightIndex++;
	});
	lightDataHeader->ambient = scene.GetAmbientLightColor();
	lightDataHeader->camPos = camtrans.GetGlobalPosition();
	lightDataHeader->count = lightIndex;

	// 3. Render shadow maps.
	if (m_directionalShadowQueue.size() != 0)
	{
		DirectionalShadowData* directionalShadows = static_cast<DirectionalShadowData*>(g_frameAllocator.Allocate(sizeof(DirectionalShadowData) * m_directionalShadowQueue.size(), alignof(DirectionalShadowData)));
		Renderer::BindFrameBuffer(&m_directionalShadowMap);
		Renderer::SetViewPort(m_directionalShadowMap->Width(), m_directionalShadowMap->Height());
		Renderer::ClearDepth(1.0f);
		uint32_t directionalShadowMapIndex = 0;
		currentMaterial = nullptr;
		for (uint32_t i = 0; i < m_directionalShadowQueue.size(); i++)
		{
			DirectionalLight& light = m_directionalShadowQueue[i];
			lightDataArray[light.lightIndex].shadowIndex = i;
			uint32_t cascadeCount = light.light->ShadowCascadeCount();
			glm::vec4 const& cascadeRatio = light.light->ShadowCascadeRatio();
			directionalShadows[i].shadowMapIndex = directionalShadowMapIndex;
			directionalShadows[i].cascadeCount = cascadeCount;
			directionalShadows[i].cascadeDepths = shadowFrustum.GetShadowDepth(cascadeRatio);
			float extension = light.light->ShadowExtension();
			FrustumCuller lightFrustum(shadowFrustum.CreateDirectional(light.direction, extension, cascadeRatio, cascadeCount, m_directionalShadowResolution, directionalShadows[i].viewProjMats, directionalShadows[i].radii));
			Renderer::UpdateBuffer(&m_directionalShadowData, i * sizeof(DirectionalShadowData), MakeStaticPointer(directionalShadows + i), sizeof(DirectionalShadowData));
			Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader) - 4, CopyPointer<true>(&i), 4);
			DrawShadowMap(currentMaterial, lightFrustum, scene);
			directionalShadowMapIndex += cascadeCount;
		}

#if GLEX_DEBUG_RENDERING
		static Optional<ShadowFrustum> viewFrustum, lightFrustums[4];
		if (Input::Pressed(Input::X))
		{
			viewFrustum.Emplace(globalData->projMat, globalData->viewProjMat);
			for (uint32_t i = 0; i < 4; i++)
				lightFrustums[i].Emplace(globalData->projMat, directionalShadows[0].viewProjMats[i]);
		}

		viewFrustum->DrawDebugLines(glm::vec3(1.0f, 0.0f, 1.0f));
		lightFrustums[0]->DrawDebugLines(glm::vec3(1.0f, 0.0f, 0.0f));
		lightFrustums[1]->DrawDebugLines(glm::vec3(0.0f, 1.0f, 0.0f));
		lightFrustums[2]->DrawDebugLines(glm::vec3(0.0f, 0.0f, 1.0f));
		lightFrustums[3]->DrawDebugLines(glm::vec3(1.0f, 1.0f, 0.0f));
#endif
	}

	// 4. Cull lights using compute shader.
	Renderer::UpdateBuffer(&m_lightData, 0, MakeStaticPointer(lightData), sizeof(LightDataHeader) + sizeof(LightData) * lightIndex);
	Renderer::BindShader(m_lightCullingShader);
	Renderer::DispatchCompute(m_horizontalTiles, m_verticalTiles, 1);

	// 5. Render scene to framebuffer.
	currentMaterial = nullptr;
	glm::vec3 const& clearColor = scene.GetClearColor();
	Renderer::BindFrameBuffer(&m_hdr);
	Renderer::ClearColor(0, clearColor.r, clearColor.g, clearColor.b, 1.0f);
	Renderer::ClearDepth(1.0f);
	Renderer::SetViewPort(Window::Width(), Window::Height());
	for (auto& [tr, mr] : m_opaqueQueue)
		DrawObject(currentMaterial, *tr, *mr);
	scene.ForEach<SkinnedMeshRenderer, Transform>([&](uint32_t id, SkinnedMeshRenderer& smr, Transform& tr)
	{
		ResPtr<Material>material = smr.Material();
		ResPtr<Mesh> mesh = smr.Mesh();
		if (material == nullptr || mesh == nullptr)
			return;
		eastl::vector<glm::mat4, Allocator> const& bones = smr.Pose();
		uint32_t size = bones.size() * sizeof(glm::mat4);
		Renderer::UpdateBuffer(&m_boneData, 0, CopyPointer<true>(bones.data(), size), size);
		if (material.GetPointer() != currentMaterial)
		{
			Renderer::BindMaterial(material, RenderPass::Color);
			currentMaterial = material.GetPointer();
		}
		glm::mat4 const& modelMat = tr.GetModelMat();
		glm::mat4 normalMat = glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMat))));
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader), CopyPointer(&modelMat), sizeof(glm::mat4));
		Renderer::UpdateBuffer(&m_globalData, sizeof(GlobalDataHeader) + sizeof(glm::mat4), CopyPointer(&normalMat), sizeof(glm::mat4));
		Renderer::BindMesh(mesh);
		Renderer::Draw();
	});
	if (!m_transparentQueue.empty())
	{
		Renderer::SetFunctionEnabled(RenderCommand::Blending, true);
		for (auto& [tr, mr, dist] : m_transparentQueue)
			DrawObject(currentMaterial, *tr, *mr);
		Renderer::SetFunctionEnabled(RenderCommand::Blending, false);
	}

#if GLEX_DEBUG_RENDERING
	DrawDebugLines();
#endif

	// 6. Render framebuffer to screen.
	m_opaqueQueue.clear();
	m_transparentQueue.clear();
	m_directionalShadowQueue.clear();
	Renderer::UnbindDrawFrameBuffer();
	Renderer::BlitFrameBuffer(Window::Width(), Window::Height());
}