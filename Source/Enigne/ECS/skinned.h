#pragma once
#include "Resource/resmgr.h"
#include "Core/Container/function.h"
#include "Engine/Component/mesh_renderer.h"
#include "Engine/Animation/animctrl.h"

namespace glex
{

class SkinnedMeshRenderer : public MeshRenderer
{
private:
	// Pose matrices must be cached because I use two-pass rendering.
	// Why don't I just put an armature here? They can even be used in parallel, hooray!
	Armature m_armature;
	SharedPtr<IAnimationController> m_controller;

public:
	SkinnedMeshRenderer(ResPtr<glex::Material> material, ResPtr<glex::Mesh> mesh, SharedPtr<IAnimationController> const& controller) :
		MeshRenderer(material, mesh), m_armature(mesh)
	{
		m_controller = controller;
	}

	SkinnedMeshRenderer(SkinnedMeshRenderer&& rhs) = default;
	SkinnedMeshRenderer& operator=(SkinnedMeshRenderer&& rhs) = default;

	void SetMesh(ResPtr<glex::Mesh> mesh)
	{
		if (m_mesh == mesh)
			return;
		ResourceManager::Release(m_mesh);
		m_mesh = mesh;
		m_armature.SetSkeleton(mesh);
	}

	SharedPtr<IAnimationController> const& Controller() const { return m_controller; }
	void SetController(SharedPtr<IAnimationController> const& controller) { m_controller = controller; }

#ifdef GLEX_INTERNAL
	eastl::vector<glm::mat4, Allocator> const& Pose() const { return m_armature.Pose(); }

	void Update(uint32_t entity, Transform const& transform)
	{
		m_controller->ApplyPose(m_armature, entity, transform);
		m_armature.Finalize();
	}
#endif
};

}