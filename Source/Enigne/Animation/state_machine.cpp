#include "state_machine.h"

using namespace glex;

void StateMachine::EnterState(AnimationState* state, float transmissionTime)
{
	m_transmissionTime = transmissionTime;
	m_timeRemaining = transmissionTime;
	m_previousState = m_currentState;
	m_currentState = state;
	state->EnterState();
}

void StateMachine::Apply(Armature& armature, float weight)
{
	if (m_timeRemaining > 0.0f)
	{
		// Blend between two states.
		m_timeRemaining -= Time::DeltaTime();
		if (m_timeRemaining < 0.0f)
		{
			m_timeRemaining = 0.0f;
			goto single_state;
		}
		else
		{
			float oldFactor = m_timeRemaining / m_transmissionTime;
			m_previousState->Apply(armature, oldFactor * weight);
			m_currentState->Apply(armature, (1.0f - oldFactor) * weight);
		}
	}
	else
	{
	single_state:
		m_currentState->Apply(armature, weight);
	}
}