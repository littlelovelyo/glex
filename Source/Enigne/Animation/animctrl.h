#pragma once
#include "armature.h"
#include "Component/transform.h"

namespace glex
{

class IAnimationController
{
public:
	virtual void ApplyPose(Armature& armature, uint32_t id, Transform const& trans) = 0;
};

}