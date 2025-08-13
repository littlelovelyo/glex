/**
 * BITMAP FONT
 * 
 * I want to render vector font in the future.
 */
#pragma once
#include "Core/Container/basic.h"
#include "Core/Container/sequence.h"
#include "Engine/Renderer/texture.h"

namespace glex::ui
{
	struct BitmapGlyph
	{
		uint32_t page;
		glm::vec4 uv;
		glm::vec2 offset;
		glm::vec2 size;
		float xadvance;
	};

	class BitmapFont : private Unmoveable
	{
	private:
		HashMap<uint32_t, BitmapGlyph> m_glyphTable;
		Vector<SharedPtr<Texture>> m_textures;
		float m_lineHeight;

		bool ReadProperty(char const*& str, char const* prop, uint32_t length, uint32_t& result);

	public:
		BitmapFont(SequenceView<SharedPtr<Texture> const> textures, char const* descFile);
		bool IsValid() const { return !m_glyphTable.empty(); }
		float LineHeight() const { return m_lineHeight; }
		SharedPtr<Texture> const& GetTexture(uint32_t page) const { return m_textures[page]; }
		BitmapGlyph const* GetGlyph(uint32_t c) const;
	};
}