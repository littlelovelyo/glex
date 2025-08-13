#include "renderer.h"

using namespace glex;
using namespace glex::impl;

void Renderer::DrawQuad(glm::vec4 const& border, glm::vec4 const& color)
{
	uint32_t currentVertexCount = s_currentQuadCount * 4;
	QuadVertex* currentVertex = s_quadVertexBuffer + currentVertexCount;
	uint32_t* currentIndex = s_quadIndexBuffer + s_currentQuadCount * 6;
	currentVertex[0].pos = glm::vec2(border.x, border.y);
	currentVertex[0].uv = glm::vec2(0.0f, 0.0f);	// Prevent NAN stuff.
	currentVertex[0].textureID = 0;	// All branches may be called!
	currentVertex[0].flags = 0;		// Solid color.
	currentVertex[0].color = color;
	currentVertex[0].slice = {};		// Prevent NAN stuff(just in case).
	currentVertex[1].pos = glm::vec2(border.x, border.y + border.w);
	currentVertex[1].uv = glm::vec2(0.0f, 0.0f);
	currentVertex[1].textureID = 0;
	currentVertex[1].flags = 0;
	currentVertex[1].color = color;
	currentVertex[1].slice = {};
	currentVertex[2].pos = glm::vec2(border.x + border.z, border.y + border.w);
	currentVertex[2].uv = glm::vec2(0.0f, 0.0f);
	currentVertex[2].textureID = 0;
	currentVertex[2].flags = 0;
	currentVertex[2].color = color;
	currentVertex[2].slice = {};
	currentVertex[3].pos = glm::vec2(border.x + border.z, border.y);
	currentVertex[3].uv = glm::vec2(0.0f, 0.0f);
	currentVertex[3].textureID = 0;
	currentVertex[3].flags = 0;
	currentVertex[3].color = color;
	currentVertex[3].slice = {};
	currentIndex[0] = currentVertexCount + 0;
	currentIndex[1] = currentVertexCount + 1;
	currentIndex[2] = currentVertexCount + 2;
	currentIndex[3] = currentVertexCount + 0;
	currentIndex[4] = currentVertexCount + 2;
	currentIndex[5] = currentVertexCount + 3;
	s_currentQuadCount++;
}

void Renderer::DrawQuad(glm::vec4 const& border, ResPtr<Texture> texture, glm::vec4 uv, glm::vec4 const& color)
{
	uint16_t texID = BindTexture(texture);
	uint32_t currentVertexCount = s_currentQuadCount * 4;
	QuadVertex* currentVertex = s_quadVertexBuffer + currentVertexCount;
	uint32_t* currentIndex = s_quadIndexBuffer + s_currentQuadCount * 6;
	currentVertex[0].pos = glm::vec2(border.x, border.y);
	currentVertex[0].uv = glm::vec2(uv.x, uv.y);
	currentVertex[0].textureID = texID;
	currentVertex[0].flags = 1;		// Plain texture.
	currentVertex[0].color = color;
	currentVertex[0].slice = {};
	currentVertex[1].pos = glm::vec2(border.x, border.y + border.w);
	currentVertex[1].uv = glm::vec2(uv.x, uv.y - uv.w);
	currentVertex[1].textureID = texID;
	currentVertex[1].flags = 1;
	currentVertex[1].color = color;
	currentVertex[1].slice = {};
	currentVertex[2].pos = glm::vec2(border.x + border.z, border.y + border.w);
	currentVertex[2].uv = glm::vec2(uv.x + uv.z, uv.y - uv.w);
	currentVertex[2].textureID = texID;
	currentVertex[2].flags = 1;
	currentVertex[2].color = color;
	currentVertex[2].slice = {};
	currentVertex[3].pos = glm::vec2(border.x + border.z, border.y);
	currentVertex[3].uv = glm::vec2(uv.x + uv.z, uv.y);
	currentVertex[3].textureID = texID;
	currentVertex[3].flags = 1;
	currentVertex[3].color = color;
	currentVertex[3].slice = {};
	currentIndex[0] = currentVertexCount + 0;
	currentIndex[1] = currentVertexCount + 1;
	currentIndex[2] = currentVertexCount + 2;
	currentIndex[3] = currentVertexCount + 0;
	currentIndex[4] = currentVertexCount + 2;
	currentIndex[5] = currentVertexCount + 3;
	s_currentQuadCount++;
}

uint16_t Renderer::BindTexture(ResPtr<Texture> texture)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if (s_textureBound[i] == texture)
			return i;
	}
	for (uint32_t i = 0; i < 32; i++)
	{
		if (s_textureBound[i] == nullptr)
		{
			Renderer::BindTexture(&*texture, i);
			s_textureBound[i] = texture;
			return i;
		}
	}
	Flush();
	BindTexture(&*texture, 0);
	s_textureBound[0] = texture;
	return 0;
}

void Renderer::Flush()
{
	GlBuffer* vertexBuffer = s_quadMesh->GetVertexBuffer();
	GlBuffer* indexBuffer = s_quadMesh->GetIndexBuffer();
	uint32_t vertexBufferSize = s_currentQuadCount * k_quadVertexStride * 4;
	uint32_t indexBufferCount = s_currentQuadCount * 6;
	uint32_t indexBufferSize = indexBufferCount * 4;
	UpdateBuffer(vertexBuffer, 0, MakeStaticPointer(s_quadVertexBuffer), vertexBufferSize);
	UpdateBuffer(indexBuffer, 0, MakeStaticPointer(s_quadIndexBuffer), indexBufferSize);
	Draw(indexBufferCount);
	s_currentQuadCount = 0;
}

void Renderer::BeginUI()
{
	memset(s_textureBound, 0, sizeof(s_textureBound));
	s_quadVertexBuffer = static_cast<QuadVertex*>(g_frameAllocator.Allocate(4 * k_quadVertexStride * k_quadBufferCount, alignof(QuadVertex)));
	s_quadIndexBuffer = static_cast<uint32_t*>(g_frameAllocator.Allocate(4 * 6 * k_quadBufferCount, alignof(uint32_t)));
	s_currentQuadCount = 0;
	BindShader(s_quadShader);
	BindMesh(s_quadMesh);
	SetFunctionEnabled(RenderCommand::DepthTest, false);
	SetFunctionEnabled(RenderCommand::Blending, true);
	SetBlendingFunction(RenderCommand::SourceAlpha, RenderCommand::OneMinusSourceAlpha, RenderCommand::Zero, RenderCommand::One);
}

void Renderer::EndUI()
{
	if (s_currentQuadCount != 0)
		Flush();
}

void Renderer::DrawControl(Control* control)
{
	glm::vec2 screen = glm::vec2(Window::Width(), Window::Height());
	control->Measure(screen);
	control->Arrange(screen);
	control->Draw(glm::vec2(0.0f, 0.0f), 1.0f);
}