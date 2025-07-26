#pragma once
#include <stdint.h>

namespace glex
{
	enum class FileAccess : uint32_t
	{
		Read,
		Write,
		ReadWrite
	};

	enum class FileOpen : uint32_t
	{
		CreateNew = 1, // CREATE_NEW
		CreateOrOverwrite = 2, // CREATE_ALWAYS
		OpenExisting = 3, // OPEN_EXISTING
		OpenOrCreate = 4, // OPEN_ALWAYS
		OverwriteExisting = 5 // TRUNCATE_EXISTING
	};

	enum class FileFlags : uint32_t
	{
		Default = 0x00000080, // FILE_ATTRIBUTE_NORMAL
		NoBuffering = 0x20000000, // FILE_FLAG_NO_BUFFERING
		RandomAccess = 0x10000000, // FILE_FLAG_RANDOM_ACCESS
		SequentialAccess = 0x08000000, // FILE_FLAG_SEQUENTIAL_SCAN
		Overlapped = 0x40000000	// FILE_FLAG_OVERLAPPED
	};

	enum class FilePosition : uint32_t
	{
		Begin = 0, // FILE_BEGIN
		Current = 1, // FILE_CURRENT
		End = 2 // FILE_END
	};
}