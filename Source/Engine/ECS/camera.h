#pragma once
#include "transform.h"

namespace glex
{
	class Camera
	{
	private:
		glm::mat4 m_projMat;

	public:
		Camera() = default;
		void Perspective(float fov, float aspect, float near, float far, bool flipY = false);
		glm::mat4 const& GetProjMat() const { return m_projMat; }
		glm::mat4 const GetViewMat(Transform const& transform) const;
	};
}