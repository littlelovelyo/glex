#include "Engine/resbase.h"
#include "Engine/resource.h"
using namespace glex;

ResourceBase::~ResourceBase()
{
	if (!m_key.empty())
		ResourceManager::FreeResource(m_key);
}