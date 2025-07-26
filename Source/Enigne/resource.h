#pragma once
#include "Core/Container/basic.h"
#include "Core/Container/function.h"
#include "Core/Memory/smart_ptr.h"
#include "Engine/Renderer/shader.h"
#include "Engine/Renderer/mesh.h"

namespace glex
{

	struct MaterialInstanceInitializer
	{
		SharedPtr<Material> material;
		uint32_t materialDomain;
		SharedPtr<Shader> shader;
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			RESOURCE MANAGER
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	class ResourceManager : private StaticClass
	{
	private:
		struct ResourceEntry
		{
		private:
			uint64_t m_type : 16;
			uint64_t m_pointer : 48;

		public:
			ResourceEntry() : m_type(0), m_pointer(0) {}
			ResourceEntry(ResourceType type, void* ptr) : m_type(*type), m_pointer(reinterpret_cast<uint64_t>(ptr)) {}
			void SetType(ResourceType type) { m_type = *type; }
			template <typename T> void SetPointer(WeakPtr<T> pointer) { m_pointer = reinterpret_cast<uint64_t>(pointer.Get()); }
			ResourceType GetType() const { return static_cast<ResourceType>(m_type); }
			template <typename T> WeakPtr<T> GetPointer() const { return reinterpret_cast<T*>(m_pointer); }
		};

		inline static HashMap<StringView, ResourceEntry> s_resourceMap;

		template <std::derived_from<ResourceBase> Res, ResourceType TYPE, typename Fn>
		static SharedPtr<Res> LoadResource(String& key, Fn&& loader)
		{
			if (key.empty())
				return nullptr;
			ResourceEntry& entry = s_resourceMap[key];
			if (entry.GetType() == ResourceType::Null)
			{
				SharedPtr<Res> resource = loader();
				if (resource == nullptr)
				{
					s_resourceMap.erase(key);
					return nullptr;
				}
				entry.SetType(TYPE);
				entry.SetPointer<Shader>(resource);
				resource->SetKey(std::move(key));
				return resource;
			}
			if (entry.GetType() == TYPE)
				return entry.GetPointer<Res>().Pin();
			return nullptr;
		}

	public:
#if GLEX_REPORT_MEMORY_LEAKS
		static void FreeMemory();
#endif
		static void FreeResource(String const& key);
		static SharedPtr<Shader> LoadShader(String key, Function<ShaderInitializer()> initializer);
		static SharedPtr<Mesh> LoadMesh(String key, Function<MeshInitializer()> initializer);
		static SharedPtr<Material> LoadMaterial(String key, Function<MaterialInitializer()> initializer);
		static SharedPtr<MaterialInstance> LoadMaterialInstance(String key, Function<MaterialInstanceInitializer()> initializer);
	};
}