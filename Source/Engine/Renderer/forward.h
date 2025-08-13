/**
 * Forward+ pipeline.
 * Reference: https://zhuanlan.zhihu.com/p/553907076.
 */
#pragma once
#include "pipeline.h"
#include "frustum.h"
#include "Graphics/shader.h"
#include "Graphics/buffer.h"
#include "Resource/mesh.h"
#include "Graphics/frame_buffer.h"
#include "Utils/optional.h"
#include "Component/light.h"
#include "Component/mesh_renderer.h"
#include "Component/transform.h"
#include <EASTL/vector.h>

namespace glex
{

class ForwardRenderer final : public Pipeline
{
protected:
	struct GlobalDataHeader
	{
		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::mat4 viewProjMat;
		glm::vec2 screenSize;
		float time;
		uint32_t currentShadow;
	};

	struct GlobalDataRest
	{
		glm::mat4 modelMat;
		glm::mat4 normalMat;
	};

	struct LightDataHeader
	{
		glm::vec3 ambient;
		float padding;
		glm::vec3 camPos;
		uint32_t count;
	};

	struct LightData
	{
		glm::vec3 color;
		float radius;
		glm::vec3 direction;
		uint16_t type;
		uint16_t shadowIndex;
	};

	struct DirectionalShadowData
	{
		uint32_t shadowMapIndex;
		uint32_t cascadeCount;
		uint64_t padding;
		glm::vec4 cascadeDepths;
		glm::vec4 radii;
		glm::mat4 viewProjMats[4];
	};

	struct DirectionalLight
	{
		Light* light;
		glm::vec3 direction;
		float intensitySquared;
		uint32_t lightIndex;
		uint32_t cascadeCount;

		static bool Compare(DirectionalLight const& lhs, DirectionalLight const& rhs)
		{
			return lhs.intensitySquared < rhs.intensitySquared;
		};
	};

	Optional<impl::FrameBuffer> m_depthMap, m_hdr, m_directionalShadowMap;
	Optional<impl::GlBuffer> m_globalData, m_boneData, m_lightData, m_tileData, m_directionalShadowData;
	// RefCountedResource<Mesh>* m_screenQuadMesh;
	ResPtr<Shader> m_lightCullingShader;
	uint32_t m_directionalShadowResolution, m_maxDirectionalShadows;
	uint32_t m_horizontalTiles, m_verticalTiles;
	uint32_t m_tileSizeX, m_tileSizeY;
	uint32_t m_maxLightCount, m_maxLightsInTile;

	struct OpaqueItem
	{
		Transform* transform;
		MeshRenderer* meshRenderer;
	};

	struct TransparentItem
	{
		Transform* transform;
		MeshRenderer* meshRenderer;
		float distance;
	};

	eastl::vector<OpaqueItem, Allocator> m_opaqueQueue;
	eastl::vector<TransparentItem, Allocator> m_transparentQueue;
	eastl::vector<DirectionalLight, Allocator> m_directionalShadowQueue;

	void DrawShadowMap(RefCountedResource<Material>*& currentMaterial, impl::FrustumCuller const& frustum, Scene& scene);
	void DrawObject(RefCountedResource<Material>*& currentMaterial, Transform& transform, MeshRenderer& meshRenderer);

#if GLEX_DEBUG_RENDERING
	inline static ResPtr<Shader> s_lineShader;
	inline static ResPtr<Mesh> s_debugLines;
	inline static uint32_t s_debugLineVertexCount = 0;
	inline static uint32_t s_debugLineIndexCount = 0;

	static void DrawDebugLines();
#endif

public:
	ForwardRenderer(uint32_t tileSizeX, uint32_t tileSizeY, uint32_t maxLightsInScene, uint32_t maxLightsInTile, uint32_t directionalShadowResolution, uint32_t maxDirectionalShadows, char const* lightShaderPath, char const* debugLineShaderPath);
	virtual ~ForwardRenderer() override;
	virtual void OnResize(uint32_t width, uint32_t height) override;
	virtual void Render(Scene& scene) override;
#if GLEX_DEBUG_RENDERING
	virtual void DrawDebugLine(glm::vec3 const* begin, glm::vec3 const* end, glm::vec3 const& color) override;
#endif
};

}