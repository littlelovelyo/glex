/**
 * Mesh file format:
 *
 * string = characters (not 0-terminated) following a 32-bit length.
 *
 * --------------------------HEADER
 *
 * uint32_t version = 0x20250422;
 * string   format = i.e. f3f3f2;    All meshes share the same format because of the limitation of our asset exporter.
 * uint32_t numMeshes;
 *
 * ------------FOR EACH MESH HEADER
 *
 * string   name;
 * uint32_t offset;
 *
 * ----------AFTER ALL MESH HEADERS
 * --------------FOR EACH MESH DATA
 *
 * uint32_t numBones;
 * vec4     boundingSphere;
 * uint32_t vertexBufferSize;
 * uint32_t indexBufferSize;
 * uint32_t compressedDataSize;
 * uint8_t  compressedData[compressedDataSize];
 */
#include "Engine/Renderer/mesh.h"
#include "Engine/Renderer/renderer.h"
#include "Core/Platform/filesync.h"
#include "Core/Container/basic.h"
#include "Core/log.h"
#include <zlib/zlib.h>

using namespace glex;

Mesh::Mesh(MeshInitializer init)
{
	{
		FileSync file(init.meshFile, FileAccess::Read, FileOpen::OpenExisting);
		if (file == nullptr)
		{
			Logger::Error("Cannot open file %s.", init.meshFile);
			return;
		}
		uint32_t magicNumber, compressedSize, numBones;
		TemporaryBuffer<Bytef> compressed = nullptr;
		TemporaryBuffer<Bytef> original = nullptr;
		uLongf actualSize;
		if (!file.Read(magicNumber) || magicNumber != 0x20250512 ||
			(m_numVertexAttributes = 0, !file.Read(reinterpret_cast<uint8_t&>(m_numVertexAttributes))) || m_numVertexAttributes == 0 || m_numVertexAttributes > Limits::NUM_VERTEX_ATTRIBUTES ||
			file.Read(m_vertexLayout, m_numVertexAttributes * sizeof(gl::DataType)) != m_numVertexAttributes * sizeof(gl::DataType) ||
			!file.Read(numBones) || numBones != 0 /* Add that in the future (or never). */ ||
			!file.Read(m_vertexBufferSize) || !file.Read(m_indexBufferSize) || m_vertexBufferSize > Limits::VERTEX_BUFFER_SIZE || m_indexBufferSize > Limits::INDEX_BUFFER_SIZE ||
			!file.Read(compressedSize) || compressedSize > Limits::VERTEX_BUFFER_SIZE + Limits::INDEX_BUFFER_SIZE ||
			(compressed = Mem::Alloc<Bytef>(compressedSize), file.Read(compressed, compressedSize) != compressedSize) ||
			(actualSize = m_vertexBufferSize + m_indexBufferSize, original = Mem::Alloc<Bytef>(actualSize), uncompress(original, &actualSize, compressed, compressedSize) != Z_OK || actualSize != m_vertexBufferSize + m_indexBufferSize))
			goto FORMAT_ERROR;

		SharedPtr<Buffer> meshBuffer = MakeShared<Buffer>(gl::BufferUsage::Vertex | gl::BufferUsage::Index | gl::BufferUsage::TransferDest, actualSize, false);
		if (!meshBuffer->IsValid())
			goto ANOTHER_ERROR;
		Renderer::UploadBuffer(meshBuffer, 0, actualSize, original);
		m_vertexBuffer = meshBuffer;
		m_vertexBufferOffset = 0;
		m_indexBuffer = std::move(meshBuffer);
		m_indexBufferOffset = m_vertexBufferSize;
		m_boundingSphere = glm::vec4(0.0f, 0.0f, 0.0f, INFINITY);
	}
	return;
FORMAT_ERROR:
	Logger::Error("File %s is not a valid mesh file.", init.meshFile);
	return;
ANOTHER_ERROR:
	Logger::Error("Cannot create mesh %s.", init.meshFile);
}

Mesh::Mesh(char const* meshFile, char const* meshName)
{
	/* {
		FileSync file(meshFile, FileAccess::Read, FileOpen::OpenExisting);
		if (file == nullptr)
		{
			Logger::Error("Cannot open file %s.", meshFile);
			return;
		}

		uint32_t version;
		if (!file.Read(version) || version != 0x20250422)
			goto FORMAT_ERROR;

		uint32_t formatLength;
		constexpr uint32_t MAX_FORMAT_LENGTH = Limits::NUM_VERTEX_ATTRIBUTES * 2;
		char format[MAX_FORMAT_LENGTH + 1];
		formatLength = file.ReadString(format, MAX_FORMAT_LENGTH);
		if (formatLength == -1 || formatLength == 0 || formatLength & 1)
			goto FORMAT_ERROR;
		m_numVertexAttributes = formatLength / 2;
		for (uint32_t i = 0; i < m_numVertexAttributes; i++)
		{
			char dataType = format[i * 2];
			int32_t numComponents = format[i * 2 + 1] - '0';
			if (numComponents < 1 || numComponents > 4)
				goto FORMAT_ERROR;
			if (dataType == 'f')
				m_vertexLayout[i] = gl::VulkanEnum::GetFormatForFloat(numComponents);
			else if (dataType == 'u')
				m_vertexLayout[i] = gl::VulkanEnum::GetFormatForInt(false, numComponents);
			else if (dataType == 'i')
				m_vertexLayout[i] = gl::VulkanEnum::GetFormatForInt(true, numComponents);
			else
				goto FORMAT_ERROR;
		}

		uint32_t supposedLength = strlen(meshName);
		uint32_t numMeshes;
		if (!file.Read(numMeshes) || numMeshes > Limits::NUM_RESOURCES)
			goto FORMAT_ERROR;
		uint32_t offset;
		for (uint32_t i = 0; i < numMeshes; i++)
		{
			char name[Limits::NAME_LENGTH + 1];
			uint32_t nameLength = file.ReadString(name, Limits::NAME_LENGTH);
			if (nameLength == -1 || !file.Read(offset))
				goto FORMAT_ERROR;
			if (nameLength == supposedLength && !strcmp(meshName, name))
				goto MESH_FOUND;
		}
		Logger::Error("File %s does not contain mesh %s.", meshFile, meshName);

	MESH_FOUND:
		if (!file.Seek(offset, FilePosition::Begin))
			goto FORMAT_ERROR;
		uint32_t compressedDataSize;
		if (!file.Read(m_numBones) || m_numBones > Limits::NUM_BONES || !file.Read(m_boundingSphere) ||
			!file.Read(m_vertexBufferSize) || !file.Read(m_indexBufferSize) || !file.Read(compressedDataSize) ||
			m_vertexBufferSize > Limits::VERTEX_BUFFER_SIZE || m_indexBufferSize > Limits::INDEX_BUFFER_SIZE || compressedDataSize > Limits::VERTEX_BUFFER_SIZE + Limits::INDEX_BUFFER_SIZE)
			goto FORMAT_ERROR;
		TemporaryBuffer<void> compressedData = Mem::Alloc(compressedDataSize);
		if (file.Read(compressedData.Get(), compressedDataSize) != compressedDataSize)
			goto FORMAT_ERROR;
		uint32_t dataSize = m_vertexBufferSize + m_indexBufferSize;
		TemporaryBuffer<void> meshData = Mem::Alloc(dataSize);
		if (uncompress(reinterpret_cast<Bytef*>(meshData.Get()), reinterpret_cast<uLongf*>(&dataSize), reinterpret_cast<Bytef*>(compressedData.Get()), compressedDataSize) != Z_OK ||
			dataSize != m_vertexBufferSize + m_indexBufferSize) // For any reason.
			goto FORMAT_ERROR;

		// Put vertex buffer and index buffer into a single buffer.
		SharedPtr<Buffer> meshBuffer = MakeShared<Buffer>(gl::BufferUsage::Vertex | gl::BufferUsage::Index | gl::BufferUsage::TransferDest, dataSize, false);
		if (!meshBuffer->IsValid())
			goto ANOTHER_ERROR;
		Renderer::UploadBuffer(meshBuffer, 0, dataSize, meshData);
		m_vertexBuffer = meshBuffer;
		m_vertexBufferOffset = 0;
		m_indexBuffer = std::move(meshBuffer);
		m_indexBufferOffset = m_vertexBufferSize;
		return;
	}
FORMAT_ERROR:
	Logger::Error("File %s is not a valid mesh file.", meshFile);
	return;
ANOTHER_ERROR:
	Logger::Error("Cannot create mesh %s.", meshName); */
}

Mesh::Mesh(void const* vertexBuffer, uint32_t vertexBufferSize, void const* indexBuffer, uint32_t indexBufferSize, SequenceView<gl::DataType const> vertexLayout, glm::vec4 const& boundingSphere)
{
	SharedPtr<Buffer> meshBuffer = MakeShared<Buffer>(gl::BufferUsage::Vertex | gl::BufferUsage::Index | gl::BufferUsage::TransferDest, vertexBufferSize + indexBufferSize, false);
	if (meshBuffer->IsValid())
	{
		Renderer::UploadBuffer(meshBuffer, 0, vertexBufferSize, vertexBuffer);
		Renderer::UploadBuffer(meshBuffer, vertexBufferSize, indexBufferSize, indexBuffer);
		m_vertexBuffer = meshBuffer;
		m_vertexBufferOffset = 0;
		m_vertexBufferSize = vertexBufferSize;
		m_indexBuffer = std::move(meshBuffer);
		m_indexBufferOffset = vertexBufferSize;
		m_indexBufferSize = indexBufferSize;
		m_boundingSphere = boundingSphere;
		m_numVertexAttributes = vertexLayout.Size();
		memcpy(m_vertexLayout, vertexLayout.Data(), sizeof(gl::DataType) * vertexLayout.Size());
	}
}

/* void Mesh::SetSkeleton(SharedPtr<Skeleton> const& skeleton)
{
	GLEX_ASSERT(IsValid() && skeleton->m_bones.size() == m_numBones) {}
	m_skeleton = skeleton;
} */

void Mesh::Draw() const
{
	gl::CommandBuffer commandBuffer = Renderer::CurrentCommandBuffer();
	commandBuffer.BindVertexBuffer(m_vertexBuffer->GetBufferObject(), m_vertexBufferOffset);
	commandBuffer.BindIndexBuffer(m_indexBuffer->GetBufferObject(), m_indexBufferOffset);
	commandBuffer.DrawIndexed(m_indexBufferSize / 4, 1, 0, 0);
}

/*————————————————————————————————————————————————————————————————————————————————————————————————————————————
		STATIC MESH MAKER
 ————————————————————————————————————————————————————————————————————————————————————————————————————————————*/
SharedPtr<Mesh> Mesh::MakeTutorialTriangle(float edge)
{
	constexpr float sqrt3 = 1.732050808f;
	constexpr float oneOverSqrt3 = 1.0f / sqrt3;
	constexpr float y0 = 0.5f * sqrt3 - 0.5f / sqrt3;
	constexpr float x2 = 0.5f;
	constexpr float y2 = -0.5f / sqrt3;
	float vertexBuffer[] =
	{
		0.0f, y0 * edge, 1.0f, 0.0f, 0.0f,
		-x2 * edge, y2 * edge, 0.0f, 1.0f, 0.0f,
		x2 * edge, y2 * edge, 0.0f, 0.0f, 1.0f
	};
	uint32_t indexBuffer[] =
	{
		0, 1, 2
	};
	return MakeShared<Mesh>(vertexBuffer, sizeof(vertexBuffer), indexBuffer, sizeof(indexBuffer), std::initializer_list { gl::DataType::Vec2, gl::DataType::Vec3 }, glm::vec4(0.0f, 0.0f, 0.0f, edge * oneOverSqrt3));
}

SharedPtr<Mesh> Mesh::MakeSkybox(float distance)
{
	constexpr float sqrt3 = 1.732050808f;
	float vertexBuffer[] =
	{
		-distance, distance, -distance,
		-distance, distance, distance,
		distance, distance, distance,
		distance, distance, -distance,
		-distance, -distance, -distance,
		-distance, -distance, distance,
		distance, -distance, distance,
		distance, -distance, -distance,
	};
	uint32_t indexBuffer[] =
	{
		0, 2, 1, 0, 3, 2,
		4, 5, 6, 4, 6, 7,
		1, 6, 5, 1, 2, 6,
		0, 4, 7, 0, 7, 3,
		1, 5, 4, 1, 4, 0,
		2, 7, 6, 2, 3, 7
	};
	return MakeShared<Mesh>(vertexBuffer, sizeof(vertexBuffer), indexBuffer, sizeof(indexBuffer), std::initializer_list { gl::DataType::Vec3 }, glm::vec4(0.0f, 0.0f, 0.0f, distance * sqrt3));
}

SharedPtr<Mesh> Mesh::MakeEarth(float radius, uint32_t latitudeSlices, uint32_t longitudeSlices)
{
	constexpr uint32_t MIN_LONGITUDE_SLICES = 3;
	constexpr uint32_t MIN_LATITUDE_SLICES = 2;
	longitudeSlices = glm::max(longitudeSlices, MIN_LONGITUDE_SLICES);
	latitudeSlices = glm::max(latitudeSlices, MIN_LATITUDE_SLICES);

	struct Vertex
	{
		glm::vec3 pos; // Normal is just a normalize(pos).
		glm::vec2 uv;
	};

	uint32_t numVertices = (latitudeSlices - 1) * (longitudeSlices + 1) + 2;
	uint32_t numTriangles = (latitudeSlices - 1) * longitudeSlices * 2;
	Vertex* vertexBuffer = Mem::Alloc<Vertex>(numVertices); // Position, normal and UV.
	uint32_t* indexBuffer = Mem::Alloc<uint32_t>(numTriangles * 3);

	// North pole.
	Vertex& first = vertexBuffer[0];
	first.pos = glm::vec3(0.0f, radius, 0.0f);
	first.uv = glm::vec2(0.5f, 1.0f);

	float deltaPhi = glm::radians(180.0f) / latitudeSlices;
	float phi = deltaPhi;
	float deltaTheta = glm::radians(360.0f) / longitudeSlices;
	uint32_t ptr = 1;
	for (uint32_t i = 1; i < latitudeSlices; i++)
	{
		float cosPhi = glm::cos(phi);
		float y = cosPhi * radius;
		float xz = glm::sin(phi) * radius;
		float theta = 0.0f;
		for (uint32_t j = 0; j <= longitudeSlices; j++)
		{
			Vertex& vertex = vertexBuffer[ptr++];
			vertex.pos.x = xz * sin(theta);
			vertex.pos.y = y;
			vertex.pos.z = xz * cos(theta);
			// Cylindrical projection.
			vertex.uv.y = cosPhi * 0.5f + 0.5f;
			vertex.uv.x = theta / glm::radians(360.0f);
			theta += deltaTheta;
		}
		phi += deltaPhi;
	}
	// South pole.
	Vertex& last = vertexBuffer[ptr];
	last.pos = glm::vec3(0.0f, -radius, 0.0f);
	last.uv = glm::vec2(0.5f, 0.0f);

	// First strip.
	ptr = 0;
	for (uint32_t i = 0; i < longitudeSlices; i++)
	{
		indexBuffer[ptr++] = 0;
		indexBuffer[ptr++] = i + 1;
		indexBuffer[ptr++] = i + 2;
	}
	// Quads.
	uint32_t baseIndex = 1;
	for (uint32_t i = 2; i < latitudeSlices; i++)
	{
		for (uint32_t j = 0; j < longitudeSlices; j++)
		{
			indexBuffer[ptr++] = baseIndex;
			indexBuffer[ptr++] = baseIndex + longitudeSlices + 1;
			indexBuffer[ptr++] = baseIndex + longitudeSlices + 2;

			indexBuffer[ptr++] = baseIndex;
			indexBuffer[ptr++] = baseIndex + longitudeSlices + 2;
			indexBuffer[ptr++] = baseIndex + 1;
			baseIndex++;
		}
		baseIndex++;
	}
	// Last strip.
	uint32_t lastIndex = numVertices - 1;
	for (uint32_t i = 0; i < longitudeSlices; i++)
	{
		indexBuffer[ptr++] = baseIndex;
		indexBuffer[ptr++] = lastIndex;
		indexBuffer[ptr++] = baseIndex + 1;
		baseIndex++;
	}

	SharedPtr<Mesh> result = MakeShared<Mesh>(vertexBuffer, numVertices * sizeof(Vertex), indexBuffer, numTriangles * 12, std::initializer_list { gl::DataType::Vec3, gl::DataType::Vec2 }, glm::vec4(0.0f, 0.0f, 0.0f, radius));
	Mem::Free(vertexBuffer);
	Mem::Free(indexBuffer);
	return result;
}