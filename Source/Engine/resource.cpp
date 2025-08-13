#include "Engine/resource.h"
#include "Engine/Renderer/shader.h"
#include "Engine/Renderer/material.h"
#include "Engine/Renderer/matinst.h"
#include "Engine/Renderer/mesh.h"
#include "Core/log.h"

using namespace glex;

#if GLEX_REPORT_MEMORY_LEAKS
void ResourceManager::FreeMemory()
{
	decltype(s_resourceMap) a;
	s_resourceMap.swap(a);
}
#endif

void ResourceManager::FreeResource(String const& key)
{
	s_resourceMap.erase(key);
}

SharedPtr<Shader> ResourceManager::LoadShader(String key, Function<ShaderInitializer()> initializer)
{
	return LoadResource<Shader, ResourceType::Shader>(key, [&]() -> SharedPtr<Shader>
	{
		ShaderInitializer init = initializer();
		if (init.vertexShaderFile == nullptr || init.fragmentShaderFile == nullptr)
			return nullptr;
		SharedPtr<Shader> shader = MakeShared<Shader>(init);
		if (!shader->IsValid())
			return nullptr;
		return shader;
	});
}

SharedPtr<Mesh> ResourceManager::LoadMesh(String key, Function<MeshInitializer()> initializer)
{
	return LoadResource<Mesh, ResourceType::Mesh>(key, [&]() -> SharedPtr<Mesh>
	{
		MeshInitializer init = initializer();
		if (init.meshFile == nullptr)
			return nullptr;
		SharedPtr<Mesh> mesh = MakeShared<Mesh>(init);
		if (!mesh->IsValid())
			return nullptr;
		return mesh;
	});
}

SharedPtr<Material> ResourceManager::LoadMaterial(String key, Function<MaterialInitializer()> initializer)
{
	if (key.empty())
		return nullptr;
	ResourceEntry& entry = s_resourceMap[key];
	if (entry.GetType() == ResourceType::Null)
	{
		MaterialInitializer param = initializer();
		if (!param.IsValid())
		{
			s_resourceMap.erase(key);
			return nullptr;
		}
		SharedPtr<Material> material = MakeShared<Material>(param);
		if (!material->IsValid())
		{
			s_resourceMap.erase(key);
			return nullptr;
		}
		entry.SetType(ResourceType::Material);
		entry.SetPointer<Material>(material);
		material->SetKey(std::move(key));
		return material;
	}
	if (entry.GetType() == ResourceType::Material)
		return entry.GetPointer<Material>().Pin();
	return nullptr;
}