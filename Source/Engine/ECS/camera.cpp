#include "Engine/ECS/camera.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glex;

void Camera::Perspective(float fov, float aspect, float near, float far, bool flipY)
{
	m_projMat = glm::perspective(fov, aspect, near, far);
	if (flipY)
		m_projMat[1][1] *= -1.0f;
}

glm::mat4 const Camera::GetViewMat(Transform const& transform) const
{
	glm::vec3 pos = transform.GetGlobalPosition();
	glm::vec3 target = pos + transform.Forward();
	return glm::lookAt(pos, target, glm::vec3(0.0f, 1.0f, 0.0f));
}