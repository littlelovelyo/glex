#pragma once
#include <glm/glm.hpp>

namespace glex
{

class Light
{
public:
	enum Type : uint16_t
	{
		Directional,
		Point,
		Spotlight
	};

private:
	glm::vec3 m_color = glm::vec3(1.0f, 1.0f, 1.0f);
	float m_radius = 1.0f;
	Type m_type = Directional;
	uint16_t m_numCascades = 0;
	glm::vec4 m_cascadeRatio = glm::vec4();
	float m_shadowExtension = 100.0f;

public:
	glm::vec3 const& Color() const { return m_color; }
	void SetColor(glm::vec3 const& color) { m_color = color; }
	float Radius() const { return m_radius; }
	void SetRadius(float radius) { m_radius = radius; }
	Type GetType() const { return m_type; }
	void SetType(Type type) { m_type = type; }
	glm::vec4 const& ShadowCascadeRatio() const { return m_cascadeRatio; }
	float ShadowExtension() const { return m_shadowExtension; }
	uint32_t ShadowCascadeCount() const { return m_numCascades; }
	void SetShadowCascadeRatio(uint32_t numCascades, glm::vec4 const& ratio) { m_numCascades = numCascades; m_cascadeRatio = ratio; }
	void SetShadowExtension(float extension) { m_shadowExtension = extension; }
};

}