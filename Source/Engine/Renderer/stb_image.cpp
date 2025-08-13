/**
 * Use heap allocation is not ideal.
 * It's better to use stack allocator.
 * But we don't have control.
 */
#include "Core/Memory/mem.h"
#include "Core/assert.h"
#define STBI_ASSERT(x) GLEX_ASSERT(x)
#define STBI_MALLOC(sz) glex::Mem::Alloc(sz)
#define STBI_REALLOC(p, newsz) glex::Mem::Realloc(p, newsz)
#define STBI_FREE(p) glex::Mem::Free(p)
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>