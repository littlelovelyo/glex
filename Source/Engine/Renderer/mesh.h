/**
 * This is a single mesh who can only have one material.
 * Storing multiple sub-meshes in a single buffer is meaningless because we will sort them by materials anyway.
 */
#pragma once
#include "Engine/Renderer/buffer.h"
#include "Core/Memory/smart_ptr.h"
#include "Core/Container/sequence.h"
#include "Core/GL/enums.h"
#include "Engine/resbase.h"
#include <array>

namespace glex
{
	struct Bone
	{
		uint32_t parent;
		glm::mat4 offset;
	};

	struct Skeleton
	{
		Vector<Bone> m_bones;
	};

	struct MeshInitializer
	{
		char const* meshFile;
	};

	class Mesh : public ResourceBase
	{
		friend class ResourceManager;

	private:
		SharedPtr<Buffer> m_vertexBuffer;
		uint32_t m_vertexBufferOffset, m_vertexBufferSize;
		SharedPtr<Buffer> m_indexBuffer;
		uint32_t m_indexBufferOffset, m_indexBufferSize;
		glm::vec4 m_boundingSphere;
		uint32_t m_numVertexAttributes;
		gl::DataType m_vertexLayout[Limits::NUM_VERTEX_ATTRIBUTES];
		SharedPtr<Skeleton> m_skeleton;

		Mesh(MeshInitializer init);
		Mesh(char const* meshFile, char const* meshName);
		Mesh(void const* vertexBuffer, uint32_t vertexBufferSize, void const* indexBuffer, uint32_t indexBufferSize, SequenceView<gl::DataType const> vertexLayout, glm::vec4 const& boundingSphere);
		bool IsValid() const { return m_vertexBuffer != nullptr; }

	public:
		void SetSkeleton(SharedPtr<Skeleton> const& skeleton);
		WeakPtr<Buffer> GetVertexBuffer() const { return m_vertexBuffer; }
		WeakPtr<Buffer> GetIndexBuffer() const { return m_indexBuffer; }
		uint32_t VertexBufferOffset() const { return m_vertexBufferOffset; }
		uint32_t VertexBufferSize() const { return m_vertexBufferSize; }
		uint32_t IndexBufferOffset() const { return m_indexBufferOffset; }
		uint32_t IndexBufferSize() const { return m_indexBufferSize; }
		glm::vec4 const& BoundingSphere() const { return m_boundingSphere; }
		void Draw() const;

		static SharedPtr<Mesh> MakeTutorialTriangle(float edge);
		static SharedPtr<Mesh> MakeSkybox(float distance);
		static SharedPtr<Mesh> MakeEarth(float radius, uint32_t latitudeSlices, uint32_t longitudeSlices);
	};
}