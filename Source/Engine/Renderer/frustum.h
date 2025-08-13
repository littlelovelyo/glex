#pragma once
#include "defs.h"
#include "config.h"
#include <glm/glm.hpp>

namespace glex::impl
{

class FrustumCuller
{
private:
	glm::vec4 m_left, m_right, m_up, m_bottom, m_near, m_far;

	static void NormalizePlane(glm::vec4& plane)
	{
		plane *= 1.0f / glm::length(lval_cast<glm::vec3>(plane));
	}

	static bool IsInside(glm::vec4 const& plane, glm::vec4 const& boundingSphere)
	{
		return glm::dot(static_cast<glm::vec3>(plane), static_cast<glm::vec3>(boundingSphere)) + plane.w + boundingSphere.w > 0;
	}

public:
	explicit FrustumCuller(glm::mat4 const& viewProjMat);
	bool IsInside(glm::vec4 const& boundingSphere) const;
};

class ShadowFrustum
{
private:
	glm::vec3 m_vertices[8];
	float m_x;

	static void CalculateDirectional(glm::vec3 const& lightDirection, float retreat, glm::mat4 const& viewMat, glm::mat4 const& invViewMat, glm::vec3 const (&corners)[8], float resolution, glm::mat4& out, float& radius);

public:
	ShadowFrustum(glm::mat4 const& projMat, glm::mat4 const& viewProjMat);
	glm::mat4 CreateDirectional(glm::vec3 const& lightDirection, float retreat, glm::vec4 const& cascadeRatio, uint32_t cascadeCount, float resolution, glm::mat4* out, glm::vec4& radii) const;
	glm::vec4 GetShadowDepth(glm::vec4 const& cascadeRatio) const;
#if GLEX_DEBUG_RENDERING
	void DrawDebugLines(glm::vec3 const& color);
#endif
};

}