/**
 * Entity component system implemented by Entt.
 *
 * Entt uses std::vector internally.
 * eastl::vector may be better.
 * But at least we can set the allocator.
 */
#pragma once
#include "Core/Memory/allocator.h"
#include "Core/Container/basic.h"
#include "Core/log.h"
 //#include "Engine/Component/transform.h"
 //#include "Engine/Component/camera.h"
 //#include "Engine/Component/script.h"
 //#include "Engine/Component/rigidbody.h"
#include "Engine/Physics/scene.h"
#include <entt/entt.hpp>

namespace glex
{
	// You can derive this class to include custom data.
	class Scene
	{
	public:
		constexpr static uint32_t NULL_ENTITY = entt::null;

	private:
		px::PhysicalScene m_pxScene;
		entt::basic_registry<uint32_t, StdAllocator<uint32_t>> m_registry; // This contains 3D stuff.

	public:
		Scene(glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f))
		{
			m_pxScene.Create(gravity);
		}

		Scene(Scene&& rhs) = default;
		Scene& operator=(Scene&& rhs) = default;

		virtual ~Scene()
		{
			m_registry.clear();
			m_pxScene.Destroy();
		}

		bool IsValid() const
		{
			return m_pxScene.GetHandle() != nullptr;
		}

		void Clear()
		{
			m_registry.clear();
		}

		uint32_t AddEntity()
		{
			uint32_t entity = m_registry.create();
			return entity;
		}

		uint32_t Count()
		{
			return m_registry.view<entt::entity>().size();
		}

		void DestroyEntity(uint32_t entity)
		{
			m_registry.destroy(entity);
		}

		template <typename Comp>
		uint32_t GetID(Comp const& comp)
		{
			return entt::to_entity(m_registry, comp);
		}

		template <typename Comp, typename... Args>
		Comp& AddComponent(uint32_t entity, Args&&... args)
		{
			return m_registry.emplace<Comp>(entity, std::forward<Args>(args)...);
		}

		template <typename Comp>
		void RemoveComponent(uint32_t entity)
		{
			m_registry.remove<Comp>(entity);
		}

		template <typename Comp>
		Comp& GetComponent(uint32_t entity)
		{
			return m_registry.get<Comp>(entity);
		}

		template <typename Comp>
		Comp* TryGetComponent(uint32_t entity)
		{
			return m_registry.try_get<Comp>(entity);
		}

		template <typename Comp0, typename Comp1, typename... Others>
		std::tuple<Comp0&, Comp1&, Others&...> GetComponent(uint32_t entity)
		{
			return m_registry.get<Comp0, Comp1, Others...>(entity);
		}

		/* template <typename Component, typename Comparator, typename Algorithm>
		void Sort(Comparator comp, Algorithm algo = InsertionSort())
		{
			m_registry.sort<MeshRenderer>(std::move(comp), std::move(algo));
			m_reorderMeshRenderers = false;
		}

		template <typename Component, typename Comparator, typename Algorithm>
		void Sort(Comparator comp, Algorithm algo = InsertionSort())
		{
			m_registry.sort<Component>(std::move(comp), std::move(algo));
		} */

		template <typename... Comp, typename Fn>
		void ForEach(Fn&& fn)
		{
			m_registry.view<Comp...>().each(std::forward<Fn>(fn));
		}

#ifdef GLEX_INTERNAL
		px::PhysicalScene GetPhysicalScene()
		{
			return m_pxScene;
		}
#endif
	};
}