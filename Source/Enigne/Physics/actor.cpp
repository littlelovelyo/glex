#include "Engine/Physics/actor.h"

using namespace glex::px;

void Actor::Destroy()
{
	if (m_actor != nullptr)
		m_actor->release();
}

PhysicalScene Actor::GetScene() const
{
	return PhysicalScene(m_actor->getScene());
}