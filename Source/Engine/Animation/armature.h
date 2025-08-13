/**
 * This is the class that manupulates the bones!
 * Animating, blending, IK, etc.
 */
#pragma once
#include "Resource/animation.h"
#include "Resource/mesh.h"
#include "Memory/allocator.h"
#include <EASTL/vector.h>
#include <glm/glm.hpp>

namespace glex
{

class Armature
{
private:
	struct IKInfo
	{
		uint32_t id;
		glm::vec3 pos;
		float length;
	};

	eastl::vector<Bone, Allocator> const* m_boneInfo = nullptr;
	eastl::vector<glm::mat4, Allocator> m_finalPose;
	static eastl::vector<IKInfo, Allocator> s_ikBuffer;

public:
	void SetSkeleton(ResPtr<Mesh> mesh);
	Armature() = default;
	Armature(ResPtr<Mesh> mesh) { SetSkeleton(mesh); }
	void BeginPose() { eastl::fill(m_finalPose.begin(), m_finalPose.end(), glm::mat4()); }
	void PushLayer(ResPtr<Animation> animation, float time, float weight);
	void IK(uint32_t end, uint32_t root, glm::vec3 const& position, glm::quat const& rotation, float tolerance, uint32_t maxIterations);
	void EndPose();
	glm::mat4 GetBoneTransform(uint32_t bone) const;
	void Finalize();
	eastl::vector<glm::mat4, Allocator> const& Pose() const { return m_finalPose; }
	uint32_t BoneCount() const { return m_finalPose.size(); }
};

}