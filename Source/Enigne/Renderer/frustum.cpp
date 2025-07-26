#include "frustum.h"
#include "Context/time.h"
#include "renderer.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glex;
using namespace glex::impl;

FrustumCuller::FrustumCuller(glm::mat4 const& viewProjMat)
{
	m_left.x = viewProjMat[0][0] + viewProjMat[0][3];
	m_left.y = viewProjMat[1][0] + viewProjMat[1][3];
	m_left.z = viewProjMat[2][0] + viewProjMat[2][3];
	m_left.w = viewProjMat[3][0] + viewProjMat[3][3];
	m_right.x = viewProjMat[0][3] - viewProjMat[0][0];
	m_right.y = viewProjMat[1][3] - viewProjMat[1][0];
	m_right.z = viewProjMat[2][3] - viewProjMat[2][0];
	m_right.w = viewProjMat[3][3] - viewProjMat[3][0];
	m_up.x = viewProjMat[0][3] - viewProjMat[0][1];
	m_up.y = viewProjMat[1][3] - viewProjMat[1][1];
	m_up.z = viewProjMat[2][3] - viewProjMat[2][1];
	m_up.w = viewProjMat[3][3] - viewProjMat[3][1];
	m_bottom.x = viewProjMat[0][1] + viewProjMat[0][3];
	m_bottom.y = viewProjMat[1][1] + viewProjMat[1][3];
	m_bottom.z = viewProjMat[2][1] + viewProjMat[2][3];
	m_bottom.w = viewProjMat[3][1] + viewProjMat[3][3];
	m_near.x = viewProjMat[0][2] + viewProjMat[0][3];
	m_near.y = viewProjMat[1][2] + viewProjMat[1][3];
	m_near.z = viewProjMat[2][2] + viewProjMat[2][3];
	m_near.w = viewProjMat[3][2] + viewProjMat[3][3];
	m_far.x = viewProjMat[0][3] - viewProjMat[0][2];
	m_far.y = viewProjMat[1][3] - viewProjMat[1][2];
	m_far.z = viewProjMat[2][3] - viewProjMat[2][2];
	m_far.w = viewProjMat[3][3] - viewProjMat[3][2];
	NormalizePlane(m_left);
	NormalizePlane(m_right);
	NormalizePlane(m_up);
	NormalizePlane(m_bottom);
	NormalizePlane(m_near);
	NormalizePlane(m_far);
}

bool FrustumCuller::IsInside(glm::vec4 const& boundingSphere) const
{
	return IsInside(m_near, boundingSphere) && IsInside(m_far, boundingSphere) &&
		IsInside(m_up, boundingSphere) && IsInside(m_bottom, boundingSphere) &&
		IsInside(m_left, boundingSphere) && IsInside(m_right, boundingSphere);
}

ShadowFrustum::ShadowFrustum(glm::mat4 const& projMat, glm::mat4 const& viewProjMat)
{
	glm::mat4 inv = glm::inverse(viewProjMat);
	for (uint32_t i = 0; i < 8; i++)
	{
		// STRANGE. REALLY.
		glm::vec4 p = inv * glm::vec4(i & 1 ? 1.0f : -1.0f, i >> 1 & 1 ? 1.0f : -1.0f, i >> 2 ? 1.0f : -1.0f, 1.0f);
		m_vertices[i] = glm::vec3(p) / p.w;
	}
	m_x = projMat[2][2];
}

glm::mat4 ShadowFrustum::CreateDirectional(glm::vec3 const& lightDirection, float retreat, glm::vec4 const& cascadeRatio, uint32_t cascadeCount, float resolution, glm::mat4* out, glm::vec4& radii) const
{
	glm::mat4 viewMat = glm::lookAt(lightDirection, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 min = glm::vec3(INFINITY, INFINITY, INFINITY), max = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
	for (uint32_t i = 0; i < 8; i++)
	{
		glm::vec3 point = viewMat * glm::vec4(m_vertices[i], 1.0f);
		min = glm::min(min, point);
		max = glm::max(max, point);
	}
	glm::mat4 projMat = glm::ortho(min.x, max.x, min.y, max.y, -max.z - retreat, -min.z);
	glm::mat4 invViewMat = glm::inverse(viewMat);
	glm::vec3 corners[8];
	for (uint32_t i = 0; i < 4; i++)
		corners[i] = m_vertices[i];
	for (uint32_t i = 0; i < 4; i++)
		corners[i + 4] = glm::mix(m_vertices[0], m_vertices[i + 4], cascadeRatio[0]);
	CalculateDirectional(lightDirection, retreat, viewMat, invViewMat, corners, resolution, out[0], radii[0]);
	for (uint32_t i = 1; i < cascadeCount; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
			corners[j] = corners[j + 4];
		for (uint32_t j = 0; j < 4; j++)
			corners[j + 4] = glm::mix(m_vertices[0], m_vertices[j + 4], cascadeRatio[i]);
		CalculateDirectional(lightDirection, retreat, viewMat, invViewMat, corners, resolution, out[i], radii[i]);
	}
	return projMat * viewMat;
}

void ShadowFrustum::CalculateDirectional(glm::vec3 const& lightDirection, float retreat, glm::mat4 const& viewMat, glm::mat4 const& invViewMat, glm::vec3 const (&corners)[8], float resolution, glm::mat4& out, float& radius)
{
	/*glm::vec3 min = glm::vec3(INFINITY, INFINITY, INFINITY), max = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
	for (uint32_t i = 0; i < 8; i++)
	{
		glm::vec3 point = viewMat * glm::vec4(corners[i], 1.0f);
		min = glm::min(min, point);
		max = glm::max(max, point);
	}
	glm::mat4 projMat = glm::ortho(min.x, max.x, min.y, max.y, -max.z - retreat, -min.z);
	out = projMat * viewMat;*/

	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < 8; i++)
		center += corners[i];
	center /= 8.0f;
	radius = 0.0f;
	for (uint32_t i = 0; i < 8; i++)
		radius = glm::max(radius, glm::distance(center, corners[i]));
	float diameter = radius * 2.0f;
	// diameter += 2.0f * diameter / (resolution - 2.0f);
	float texelUnit = resolution / diameter;
	glm::vec4 snappedCenter = viewMat * glm::vec4(center, 1.0f);
	snappedCenter.x = glm::round(snappedCenter.x * texelUnit) / texelUnit;
	snappedCenter.y = glm::round(snappedCenter.y * texelUnit) / texelUnit;
	snappedCenter = invViewMat * snappedCenter;
	center = snappedCenter;
	glm::mat4 projMat = glm::ortho(-radius, radius, -radius, radius, -radius - retreat, radius);
	out = projMat * glm::lookAt(center, center - lightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec4 ShadowFrustum::GetShadowDepth(glm::vec4 const& cascadeRatio) const
{
	/*
	x = projMat[2][2]
	y = projMat[3][2]
	near = y / (x - 1)
	far = y / (x + 1)

	(far - near) * ratio + near = near * far / (far + depth * (near - far))
	depth = r(x-1)/(-2r+x+1)
	*/
	return cascadeRatio * (m_x - 1.0f) / (-2.0f * cascadeRatio + m_x + 1.0f);
}

#if GLEX_DEBUG_RENDERING
void ShadowFrustum::DrawDebugLines(glm::vec3 const& color)
{
	/* Renderer::DrawDebugLine(m_vertices[0], m_vertices[1], color);
	Renderer::DrawDebugLine(m_vertices[0], m_vertices[2], color);
	Renderer::DrawDebugLine(m_vertices[1], m_vertices[3], color);
	Renderer::DrawDebugLine(m_vertices[2], m_vertices[3], color);

	Renderer::DrawDebugLine(m_vertices[4], m_vertices[5], color);
	Renderer::DrawDebugLine(m_vertices[4], m_vertices[6], color);
	Renderer::DrawDebugLine(m_vertices[5], m_vertices[7], color);
	Renderer::DrawDebugLine(m_vertices[6], m_vertices[7], color);

	Renderer::DrawDebugLine(m_vertices[0], m_vertices[4], color);
	Renderer::DrawDebugLine(m_vertices[1], m_vertices[5], color);
	Renderer::DrawDebugLine(m_vertices[2], m_vertices[6], color);
	Renderer::DrawDebugLine(m_vertices[3], m_vertices[7], color); */
}
#endif