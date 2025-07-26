#include "animstate.h"

using namespace glex;

void AnimationState::EnterState()
{
	if (m_type == e_SimpleState)
		reinterpret_cast<SimpleState*>(this)->EnterState();
	else
		reinterpret_cast<BlendState*>(this)->EnterState();
}

void AnimationState::Apply(Armature& armature, float weight)
{
	if (m_type == e_SimpleState)
		reinterpret_cast<SimpleState*>(this)->Apply(armature, weight);
	else
		reinterpret_cast<BlendState*>(this)->Apply(armature, weight);
}

void BlendState::Apply(Armature& armature, float weight)
{
	m_time += Time::DeltaTime();
	BlendNode* right = eastl::lower_bound(m_animations.begin(), m_animations.end(), m_value, [](BlendNode const& node, float value)
	{
		return node.value < value;
	});
	if (right == m_animations.begin())
		armature.PushLayer(right->animation, glm::mod(m_time, right->animation->duration), weight);
	else if (right == m_animations.end())
		armature.PushLayer(m_animations.back().animation, glm::mod(m_time, m_animations.back().animation->duration), weight);
	else
	{
		BlendNode* left = right - 1;
		float rightFactor = weight * (m_value - left->value) / (right->value - left->value);
		armature.PushLayer(left->animation, glm::mod(m_time, left->animation->duration), weight - rightFactor);
		armature.PushLayer(right->animation, glm::mod(m_time, right->animation->duration), rightFactor);
	}
}