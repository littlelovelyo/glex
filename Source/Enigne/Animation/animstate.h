#pragma once
#include "armature.h"
#include "Context/time.h"
#include "Resource/resmgr.h"
#include <EASTL/vector.h>

namespace glex
{

class AnimationState
{
public:
	enum Type : uint32_t
	{
		e_SimpleState,
		e_LinearBlend
	};

private:
	Type m_type;

protected:
	AnimationState(Type type) : m_type(type) {}

public:
	Type Type() const { return m_type; }
	void EnterState(); // Avoid using virtual functions.
	void Apply(Armature& armature, float weight);
};

struct SimpleState : public AnimationState
{
private:
	ResPtr<Animation> m_animation = nullptr;
	float m_time = 0.0f;
	bool m_loop = true;

public:
	SimpleState() : AnimationState(AnimationState::e_SimpleState) {}

	SimpleState(ResPtr<Animation> animation) : AnimationState(AnimationState::e_SimpleState), m_animation(animation) {}

	~SimpleState()
	{
		ResourceManager::Release(m_animation);
	}

	void SetAnimation(ResPtr<Animation> animation)
	{
		if (m_animation == animation)
			return;
		ResourceManager::Release(m_animation);
		m_animation = animation;
	}

	void SetLoop(bool loop)
	{
		m_loop = loop;
	}

	float TimeRemaining() const
	{
		return m_animation->duration - m_time;
	}

#ifdef GLEX_INTERNAL
	void EnterState()
	{
		m_time = 0.0f;
	}

	void Apply(Armature& armature, float weight)
	{
		if (m_loop)
			m_time = glm::mod(m_time + Time::DeltaTime(), m_animation->duration);
		else
			m_time += Time::DeltaTime();
		armature.PushLayer(m_animation, m_time, weight);
	}
#endif
};

struct BlendNode
{
	ResPtr<Animation> animation;
	float value;
};

struct BlendState : public AnimationState
{
private:
	eastl::vector<BlendNode, Allocator> m_animations;
	float m_value = 0.0f;
	float m_time = 0.0f;

public:
	BlendState() : AnimationState(AnimationState::e_LinearBlend) {}

	BlendState(std::initializer_list<BlendNode> const& nodes) : AnimationState(AnimationState::e_LinearBlend), m_animations(nodes.begin(), nodes.end()) {}

	~BlendState()
	{
		for (BlendNode const& node : m_animations)
			ResourceManager::Release(node.animation);
	}

	void SetAnimations(std::initializer_list<BlendNode> const& nodes)
	{
		for (BlendNode const& node : m_animations)
			ResourceManager::Release(node.animation);
		m_animations.clear();
		m_animations.insert(m_animations.end(), nodes.begin(), nodes.end());
	}

	void SetValue(float value)
	{
		m_value = value;
	}

#ifdef GLEX_INTERNAL
	void EnterState()
	{
		m_time = 0.0f;
	}

	void Apply(Armature& armature, float weight);
#endif
};

}