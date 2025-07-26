#pragma once
#include "Core/commdefs.h"
#include "Core/Container/basic.h"
#include "Core/Container/nullable.h"

namespace glex
{
	struct Utf8Code
	{
		uint32_t code;
		uint32_t length;
	};

	struct Utf8Char
	{
		char str[4];
		uint32_t length;
	};

	class StringUtils : private StaticClass
	{
	public:
		// String format
		static Nullable<String> Format(char const* format, ...);
		static Nullable<String> Format(char const* format, va_list ap);
		static char const* Format(char* buffer, uint32_t size, char const* format, ...);
		static char const* Format(char* buffer, uint32_t size, char const* format, va_list ap);
		static char* FormatAutoExpand(char* buffer, uint32_t size, char const* format, ...);
		static char* FormatAutoExpand(char* buffer, uint32_t size, char const* format, va_list ap);

		template <uint32_t SIZE>
		static char const* Format(char(&buffer)[SIZE], char const* format, ...)
		{
			va_list ap;
			va_start(ap, format);
			Format(buffer, SIZE, ap);
			va_end(ap);
		}

		template <uint32_t SIZE>
		static char* FormatAutoExpand(char(&buffer)[SIZE], char const* format, ...)
		{
			va_list ap;
			va_start(ap, format);
			FormatAutoExpand(buffer, SIZE, ap);
			va_end(ap);
		}

		// UTF-8 reader
		static bool IsASCII(char chr) { return (chr & 0x80) == 0; }
		static Utf8Code CodeOfUtf8(char const* utf8Char);
		static Utf8Char Utf8OfCode(uint32_t code);
		static bool IsValidUtf8(char const* string);

		// Code coverter
		static Nullable<WideString> Utf16Of(char const* string);
		static Nullable<String> Utf8Of(wchar_t const* string);
		static wchar_t const* Utf16Of(wchar_t* buffer, uint32_t size, char const* string);
		static char const* Utf8Of(char* buffer, uint32_t size, wchar_t const* string);
		static wchar_t* Utf16OfAutoExpand(wchar_t* buffer, uint32_t size, char const* string);
		static char* Utf8OfAutoExpand(char* buffer, uint32_t size, wchar_t const* string);

		template <uint32_t SIZE>
		static wchar_t const* Utf16Of(wchar_t(&buffer)[SIZE], char const* string)
		{
			return Utf16Of(buffer, SIZE, string);
		}

		template <uint32_t SIZE>
		static char const* Utf8Of(char(&buffer)[SIZE], wchar_t const* string)
		{
			return Utf8Of(buffer, SIZE, string);
		}

		template <uint32_t SIZE>
		static wchar_t* Utf16OfAutoExpand(wchar_t(&buffer)[SIZE], char const* string)
		{
			return Utf16OfAutoExpand(buffer, size, string);
		}

		template <uint32_t SIZE>
		static char* Utf8OfAutoExpand(char(&buffer)[SIZE], wchar_t const* string)
		{
			return Utf8OfAutoExpand(buffer, size, string);
		}

		// Value parser
		static char const* Parse(char const* str, uint32_t& result);
		static uint32_t ToString(uint32_t value, char* buffer); // Not null-terminated.
		static uint32_t ToString(uint32_t value, char* buffer, uint32_t size); // Not null-terminated. Return value: length. 0 on error.
		static String ToString(float value, uint32_t precision = 7);
		static String ToString(uint32_t value);
		static String ToString(int32_t value);

		// Basic operation
		static char* Copy(char const* str);
		static bool Append(char* dest, uint32_t size, char const* src);
		static void Replace(char* str, char from, char to);

		template <uint32_t SIZE>
		static bool Append(char(&dest)[SIZE], char const* src)
		{
			return Append(dest, SIZE, src);
		}

		// Path manipulation
		static char const* GetFileName(char const* fullPath);
		static char const* GetOuterPath(char const* path, char* buffer, uint32_t size);
		static void GetOuterPath(char* path);
		static String JoinPath(StringView path, StringView folder);
		static uint32_t JoinPath(char* path, uint32_t size, StringView folder); // Join in-place. Length returned. 0 on error.
		static uint32_t JoinPath(char* buffer, uint32_t size, StringView path, StringView folder);		

		template <uint32_t SIZE>
		static uint32_t JoinPath(char(&dest)[SIZE], StringView src)
		{
			return JoinPath(dest, SIZE, src);
		}

		template <uint32_t SIZE>
		static uint32_t JoinPath(char(&buffer)[SIZE], StringView path, StringView folder)
		{
			return JoinPath(buffer, SIZE, path, folder);
		}
	};
}