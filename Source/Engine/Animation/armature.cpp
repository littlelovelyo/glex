#include "armature.h"
#include "Resource/animation.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Renderer/renderer.h"

using namespace glex;

template <typename Key>
static auto Interpolate(eastl::vector<Key, Allocator> const& list, float time, auto method)
{
	Key const* next = eastl::upper_bound(list.begin(), list.end(), time, [](float t, Key const& k) { return t < k.time; });
	if (next == list.begin())
		return next->value;
	if (next == list.end())
		return list.back().value;
	Key const* prev = next - 1;
	return method(prev->value, next->value, (time - prev->time) / (next->time - prev->time));
}

void Armature::SetSkeleton(ResPtr<Mesh> mesh)
{
	if (mesh == nullptr)
		return;
	m_boneInfo = &mesh->GetBones();
	m_finalPose.resize(m_boneInfo->size());
}

void Armature::PushLayer(ResPtr<Animation> animation, float time, float weight)
{
	GLEX_LASSERT(animation->bones <= m_finalPose.size()) {}
	constexpr glm::vec3(&mix)(glm::vec3 const&, glm::vec3 const&, float) = glm::mix;
	constexpr glm::quat(&slerp)(glm::quat const&, glm::quat const&, float) = glm::slerp;
	for (BoneSequence const& seq : animation->sequences)
	{
		glm::vec3 position = Interpolate(seq.positions, time, mix);
		glm::quat rotation = Interpolate(seq.rotations, time, slerp);
		glm::vec3 scale = Interpolate(seq.scales, time, mix);
		glm::mat4 transform = glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
		transform[3] = glm::vec4(position, 1.0f);
		m_finalPose[seq.id] += transform * weight;
	}
}

// FABRIK algorithm.
void Armature::IK(uint32_t end, uint32_t root, glm::vec3 const& position, glm::quat const& rotation, float tolerance, uint32_t maxIterations)
{
	// Make sure we have at least two bones.
	GLEX_LASSERT(end != root) {}
	glm::mat4 baseTransform = root == 0 ? glm::mat4(1.0f) : GetBoneTransform((*m_boneInfo)[root].parent);
	glm::vec3 rootPos = (baseTransform * m_finalPose[root])[3];
	s_ikBuffer.clear();
	// Maybe cache somewhere... a class like 'IKDescription' or something.
	for (uint32_t b = end; b != root; b = (*m_boneInfo)[b].parent)
	{
		glm::vec3 translation = m_finalPose[b][3];
		// GetBoneTransform here can be highly optimized.
		s_ikBuffer.push_back({ b, glm::vec3(), glm::length(translation)});
	}
	uint32_t numArticulations = s_ikBuffer.size();
	// Sentinel.
	s_ikBuffer.push_back({ root, rootPos, 0.0f});
	glm::mat4 currentTransform = baseTransform;
	// Now we'are optimizing.
	for (uint32_t b = numArticulations; b <= numArticulations; b--)
	{
		currentTransform = currentTransform * m_finalPose[s_ikBuffer[b].id];
		s_ikBuffer[b].pos = currentTransform[3];
	}
	for (uint32_t i = 0; i < maxIterations; i++)
	{
		glm::vec3 endPos = s_ikBuffer[0].pos;
		float error = glm::distance(endPos, position);
		if (error <= tolerance)
			break;
		// We put the end bone directly to its position.
		s_ikBuffer[0].pos = position;
		for (uint32_t b = 1; b < numArticulations; b++)
		{
			// Then for each joint following, correct its position.
			glm::vec3 const& prevPos = s_ikBuffer[b - 1].pos;
			glm::vec3& nextPos = s_ikBuffer[b].pos;
			nextPos = prevPos + s_ikBuffer[b - 1].length * glm::normalize(nextPos - prevPos);
		}
		for (uint32_t b = numArticulations - 1; b < numArticulations; b--)
		{
			glm::vec3 const& prevPos = s_ikBuffer[b + 1].pos;
			glm::vec3& nextPos = s_ikBuffer[b].pos;
			nextPos = prevPos + s_ikBuffer[b].length * glm::normalize(nextPos - prevPos);
		}
		float correction = glm::distance(endPos, s_ikBuffer[0].pos);
		if (correction < tolerance)
			break;
	}
	// Now we get all the positions.
	// But we need to convert them back to local space.
	for (uint32_t b = numArticulations; b > 0; b--)
	{
		glm::vec3 const& currentPos = s_ikBuffer[b].pos;
		glm::vec3 const& nextPos = s_ikBuffer[b - 1].pos;
		glm::quat rot(glm::vec3(0.0f, 1.0f, 0.0f), (nextPos - currentPos) / s_ikBuffer[b - 1].length);
		glm::mat4 transform = glm::mat4_cast(rot);
		transform[3] = glm::vec4(currentPos, 1.0f);
		// What about scale?
		m_finalPose[s_ikBuffer[b].id] = glm::inverse(baseTransform) * transform;
		baseTransform = transform;
	}
	// Now take care of end bone's transform.
	glm::mat4 transform = glm::mat4_cast(rotation);
	// glm::mat4 transform = m_finalPose[end];
	transform[3] = glm::vec4(s_ikBuffer[0].pos, 1.0f);
	m_finalPose[end] = glm::inverse(baseTransform) * transform;
	// Yay! We're done! First Try! Hooray!
}

void Armature::EndPose()
{
	for (uint32_t i = 1; i < m_boneInfo->size(); i++)
		m_finalPose[i] = m_finalPose[(*m_boneInfo)[i].parent] * m_finalPose[i];
}

glm::mat4 Armature::GetBoneTransform(uint32_t bone) const
{
	glm::mat4 result = m_finalPose[bone];
	while (bone != 0)
	{
		bone = (*m_boneInfo)[bone].parent;
		result = m_finalPose[bone] * result;
	}
	return result;
}

void Armature::Finalize()
{
	for (uint32_t i = 0; i < m_boneInfo->size(); i++)
		m_finalPose[i] = m_finalPose[i] * (*m_boneInfo)[i].offset;
}