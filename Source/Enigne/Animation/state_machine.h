#pragma once
#include "Component/transform.h"
#include "Memory/allocator.h"
#include "animstate.h"
#include <EASTL/vector.h>
#include <glm/glm.hpp>

namespace glex
{

class StateMachine
{
private:
	AnimationState* m_previousState = nullptr;
	AnimationState* m_currentState = nullptr;
	float m_transmissionTime = 0.0f;
	float m_timeRemaining = 0.0f;

public:
	StateMachine(AnimationState* initialState) : m_currentState(initialState) {}
	void EnterState(AnimationState* state, float transmissionTime);
	AnimationState* CurrentState() const { return m_currentState; }
	void Apply(Armature& armature, float weight);
};

}