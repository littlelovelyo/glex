#pragma once
#include "Core/Container/basic.h"

namespace glex
{
	class Shader;
	class Material;
	class MaterialInstance;
	class MaterialInitializer;
	class Mesh;

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
				TYPE INFO
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	enum class ResourceType : uint16_t
	{
		Null,
		Shader,
		Material,
		MaterialInstance,
		Mesh
	};

	/*————————————————————————————————————————————————————————————————————————————————————————————————————
			RESOURCE BASE CLASS
	————————————————————————————————————————————————————————————————————————————————————————————————————*/
	class ResourceManager;

	class ResourceBase : private Unmoveable
	{
		friend class ResourceManager;

	private:
		String m_key;

		void SetKey(String key) { m_key = std::move(key); }

	public:
		~ResourceBase();
		String const& GetKey() const { return m_key; }
	};
}