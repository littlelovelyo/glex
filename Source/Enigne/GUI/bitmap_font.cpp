#include "Engine/GUI/bitmap_font.h"
#include "Core/Platform/filesync.h"
#include "Core/Utils/string.h"
#include "Core/Utils/raii.h"
#include "Core/log.h"

using namespace glex::ui;

bool BitmapFont::ReadProperty(char const*& str, char const* property, uint32_t length, uint32_t& result)
{
	str = strstr(str, property);
	if (str == nullptr)
		return false;
	str = StringUtils::Parse(str + length, result);
	return true;
}

// If font has multiple pages, this may not work. Be careful!
BitmapFont::BitmapFont(SequenceView<SharedPtr<Texture> const> textures, char const* descFile)
{
	for (SharedPtr<Texture> const& texture : textures)
	{
		if (!texture->IsValid())
			return;
	}

	FileSync file(descFile, FileAccess::Read, FileOpen::OpenExisting);
	if (file == nullptr)
	{
		Logger::Error("Cannot open %s.", descFile);
		return;
	}
	TemporaryBuffer<char> buffer = Mem::Alloc<char>(file.Size() + 1);
	if (file.Read(buffer, file.Size()) != file.Size())
	{
		Logger::Error("Cannot read %s.", descFile);
		return;
	}
	buffer[file.Size()] = 0;

	char const* p = buffer;
	uint32_t size, lineHeight, textureWidth, textureHeight, numPages, currentPage;
	if (!ReadProperty(p, "size=", 5, size) ||
		!ReadProperty(p, "lineHeight=", 11, lineHeight) ||
		!ReadProperty(p, "scaleW=", 7, textureWidth) ||
		!ReadProperty(p, "scaleH=", 7, textureHeight) ||
		!ReadProperty(p, "pages=", 6, numPages) || numPages != textures.Size())
	{
		Logger::Error("%s is not a valid font file.", descFile);
		return;
	}
	CancellableAutoCleaner hashTableCleaner([this]()
	{
		decltype(m_glyphTable) x;
		m_glyphTable.swap(x);
	});
	for (uint32_t pg = 0; pg < numPages; pg++)
	{
		uint32_t numChars;
		if (!ReadProperty(p, "page id=", 8, currentPage) || currentPage >= numPages ||
			!ReadProperty(p, "chars count=", 12, numChars))
		{
			Logger::Error("%s is not a valid font file.", descFile);
			return;
		}
		for (uint32_t cr = 0; cr < numChars; cr++)
		{
			uint32_t code, x, y, width, height, xoffset, yoffset, xadvance;
			if (!ReadProperty(p, "char id=", 8, code) ||
				!ReadProperty(p, "x=", 2, x) ||
				!ReadProperty(p, "y=", 2, y) ||
				!ReadProperty(p, "width=", 6, width) ||
				!ReadProperty(p, "height=", 7, height) ||
				!ReadProperty(p, "xoffset=", 8, xoffset) ||
				!ReadProperty(p, "yoffset=", 8, yoffset) ||
				!ReadProperty(p, "xadvance=", 9, xadvance))
			{
				Logger::Error("%s is not a valid font file.", descFile);
				return;
			}
			BitmapGlyph glyph;
			glyph.page = currentPage;
			glyph.uv = glm::vec4(x / static_cast<float>(textureWidth), 1.0f - y / static_cast<float>(textureHeight),
				width / static_cast<float>(textureWidth), height / static_cast<float>(textureHeight));
			glyph.offset = glm::vec2(xoffset / static_cast<float>(size), yoffset / static_cast<float>(size));
			glyph.size = glm::vec2(width / static_cast<float>(size), height / static_cast<float>(size));
			glyph.xadvance = xadvance / static_cast<float>(size);
			m_glyphTable[code] = glyph;
		}
	}
	m_lineHeight = lineHeight / static_cast<float>(size);
	m_textures.insert(m_textures.end(), textures.begin(), textures.end());
	hashTableCleaner.Cancel();
}

BitmapGlyph const* BitmapFont::GetGlyph(uint32_t c) const
{
	auto iter = m_glyphTable.find(c);
	if (iter == m_glyphTable.end())
		return nullptr;
	return &(*iter).second;
}