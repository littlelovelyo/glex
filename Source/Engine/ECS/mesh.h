/**
 * Maybe it's better to make mesh renderer a separate component,
 * because we want to make render pipeline highly customizable.
 * 
 * No. That will be impossible to serialize. Use this MeshRenderer.
 */
#pragma once
#include "Engine/Renderer/mesh.h"
#include "Engine/Renderer/matinst.h"

namespace glex
{
	enum class RenderOrder : uint8_t
	{
		Opaque,
		Transparent,
	};

	class MeshRenderer
	{
	protected:
		bool m_castShadow = true;
		RenderOrder m_order = RenderOrder::Opaque;
		SharedPtr<Mesh> m_mesh;
		Vector<SharedPtr<MaterialInstance>> m_materials;

	public:
		MeshRenderer() = default;

		MeshRenderer(MeshRenderer&& rhs) :
			m_materials(std::move(rhs.m_materials)), m_mesh(std::move(rhs.m_mesh)), m_castShadow(rhs.m_castShadow), m_order(rhs.m_order) {}

		MeshRenderer& operator=(MeshRenderer&& rhs)
		{
			m_materials = std::move(rhs.m_materials);
			m_mesh = std::move(rhs.m_mesh);
			m_castShadow = rhs.m_castShadow;
			m_order = rhs.m_order;
			return *this;
		}

		void SetMesh(SharedPtr<Mesh> const& mesh)
		{
			m_mesh = mesh;
		}

		void SetMaterials(SequenceView<SharedPtr<MaterialInstance> const* const> materials)
		{
			m_materials.resize(materials.Size());
			m_materials.shrink_to_fit();
			for (uint32_t i = 0; i < materials.Size(); i++)
				m_materials[i] = *materials[i];
		}

		void SetMaterials(SequenceView<SharedPtr<MaterialInstance> const> materials)
		{
			m_materials.resize(materials.Size());
			m_materials.shrink_to_fit();
			for (uint32_t i = 0; i < materials.Size(); i++)
				m_materials[i] = materials[i];
		}

		SharedPtr<Mesh> GetMesh() const { return m_mesh; }
		SharedPtr<MaterialInstance> const& GetMaterial(uint32_t pass) const { return m_materials[pass]; }
		Vector<SharedPtr<MaterialInstance>> const& GetMaterials() const { return m_materials; }
		void SetCastShadow(bool castShadow) { m_castShadow = castShadow; }
		bool CastShadow() const { return m_castShadow; }
		RenderOrder GetOrder() const { return m_order; }
	};
}