#include "Core/Utils/string.h"
#include <algorithm>
#include <Windows.h>

using namespace glex;

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		STRING FORMAT
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
Nullable<String> StringUtils::Format(char const* format, ...)
{
	va_list ap;
	va_start(ap, format);
	Nullable<String> result = Format(format, ap);
	va_end(ap);
	return result;
}

Nullable<String> StringUtils::Format(char const* format, va_list ap)
{
	int32_t length = vsnprintf(nullptr, 0, format, ap);
	if (length < 0)
		return nullptr;
	String string(length, 0);
	if (vsnprintf(string.data(), length + 1, format, ap) < length)
		return nullptr;
	return string;
}

char const* StringUtils::Format(char* buffer, uint32_t size, char const* format, ...)
{
	va_list ap;
	va_start(ap, format);
	char const* result = Format(buffer, size, format, ap);
	va_end(ap);
	return result;
}

char const* StringUtils::Format(char* buffer, uint32_t size, char const* format, va_list ap)
{
	int32_t length = vsnprintf(buffer, size, format, ap);
	if (length < 0)
		return nullptr;
	if (length < size)
		return buffer;
	else
		return nullptr;
}

char* StringUtils::FormatAutoExpand(char* buffer, uint32_t size, char const* format, ...)
{
	va_list ap;
	va_start(ap, format);
	char* result = FormatAutoExpand(buffer, size, format, ap);
	va_end(ap);
	return result;
}

char* StringUtils::FormatAutoExpand(char* buffer, uint32_t size, char const* format, va_list ap)
{
	int32_t length = vsnprintf(buffer, size, format, ap);
	if (length < 0)
		return nullptr;
	if (length < size)
		return buffer;
	char* largerBuffer = Mem::Alloc<char>(length + 1);
	length = vsnprintf(largerBuffer, length + 1, format, ap);
	if (length >= 0)
		return largerBuffer;
	Mem::Free(largerBuffer);
	return nullptr;
}

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		UTF-8 READER
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
Utf8Code StringUtils::CodeOfUtf8(char const* utf8Char)
{
	char b0 = utf8Char[0];
	if ((b0 & 0x80) == 0)    // ASCII.
		return { static_cast<uint32_t>(b0), 1 };
	if ((b0 & 0xe0) == 0xc0) // 2 bytes.
	{
		char b1 = utf8Char[1];
		return { static_cast<uint32_t>(b0 & 0x1f) << 6 | (b1 & 0x3f), 2 };
	}
	if ((b0 & 0xf0) == 0xe0) // 3 bytes.
	{
		char b1 = utf8Char[1];
		char b2 = utf8Char[2];
		return { static_cast<uint32_t>(b0 & 0x0f) << 12 | (b1 & 0x3f) << 6 | (b2 & 0x3f), 3 };
	}
	if ((b0 & 0xf8) == 0xf0) // 4 bytes.
	{
		char b1 = utf8Char[1];
		char b2 = utf8Char[2];
		char b3 = utf8Char[3];
		return { static_cast<uint32_t>(b0 & 0x70) << 18 | (b1 & 0x3f) << 12 | (b2 & 0x3f) << 6 | (b3 & 0x3f), 4 };
	}
	return { 0, 0 };
}

/* Utf8Code StringUtils::CodeOfUtf8WithErrorChecking(char const* utf8Char)
{
	char b0 = utf8Char[0];
	if ((b0 & 0x80) == 0)    // ASCII.
		return { static_cast<uint32_t>(b0), 1 };
	if ((b0 & 0xe0) == 0xc0) // 2 bytes.
	{
		char const& b1 = utf8Char[1];
		if ((b1 & 0xc0) != 0x80)
			return { 0, 0 };
		return { static_cast<uint32_t>(b0 & 0x1f) << 6 | (b1 & 0x3f), 2 };
	}
	if ((b0 & 0xf0) == 0xe0) // 3 bytes.
	{
		char const& b1 = utf8Char[1];
		char const& b2 = utf8Char[2];
		if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80)
			return { 0, 0 };
		return { static_cast<uint32_t>(b0 & 0x0f) << 12 | (b1 & 0x3f) << 6 | (b2 & 0x3f), 3 };
	}
	if ((b0 & 0xf8) == 0xf0) // 4 bytes.
	{
		char const& b1 = utf8Char[1];
		char const& b2 = utf8Char[2];
		char const& b3 = utf8Char[3];
		if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80 || (b3 & 0xc0) != 0x80)
			return { 0, 0 };
		return { static_cast<uint32_t>(b0 & 0x70) << 18 | (b1 & 0x3f) << 12 | (b2 & 0x3f) << 6 | (b3 & 0x3f), 4 };
	}
	return { 0, 0 };
} */

Utf8Char StringUtils::Utf8OfCode(uint32_t code)
{
	if (code < 0x80)
		return { { static_cast<char>(code), 0, 0, 0 }, 1 };
	if (code < 0x800)
	{
		return { { static_cast<char>(0xc0 | code >> 6),
			static_cast<char>(0x80 | code & 0x3f), 0, 0 }, 2 };
	}
	if (code < 0x10000)
	{
		return { { static_cast<char>(0xe0 | code >> 12),
			static_cast<char>(0x80 | code >> 6 & 0x3f),
			static_cast<char>(0x80 | code & 0x3f), 0 }, 3 };
	}
	if (code < 0x200000)
	{
		return { { static_cast<char>(0xe0 | code >> 18),
			static_cast<char>(0x80 | code >> 12 & 0x3f),
			static_cast<char>(0x80 | code >> 6 & 0x3f),
			static_cast<char>(0x80 | code & 0x3f) }, 4 };
	}
	return { 0, 0 };
}

static uint32_t LengthOfUtf8Char(char const* utf8Char)
{
	char b0 = utf8Char[0];
	if ((b0 & 0x80) == 0)    // ASCII.
		return 1;
	if ((b0 & 0xe0) == 0xc0) // 2 bytes.
	{
		char const& b1 = utf8Char[1];
		if ((b1 & 0xc0) != 0x80)
			return 0;
		return 2;
	}
	if ((b0 & 0xf0) == 0xe0) // 3 bytes.
	{
		char const& b1 = utf8Char[1];
		char const& b2 = utf8Char[2];
		if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80)
			return 0;
		return 3;
	}
	if ((b0 & 0xf8) == 0xf0) // 4 bytes.
	{
		char const& b1 = utf8Char[1];
		char const& b2 = utf8Char[2];
		char const& b3 = utf8Char[3];
		if ((b1 & 0xc0) != 0x80 || (b2 & 0xc0) != 0x80 || (b3 & 0xc0) != 0x80)
			return 0;
		return 4;
	}
	return 0;
}

bool StringUtils::IsValidUtf8(char const* string)
{
	while (*string != 0)
	{
		uint32_t charLength = LengthOfUtf8Char(string);
		if (charLength == 0)
			return false;
		string += charLength;
	}
	return true;
}

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		CODE CONVERTER
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
Nullable<WideString> StringUtils::Utf16Of(char const* str)
{
	int count = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, nullptr, 0);
	if (count == 0)
		return nullptr;
	WideString result(count - 1, 0);
	int ret = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, result.data(), count);
	return ret != 0 ? result : nullptr;
}

Nullable<String> StringUtils::Utf8Of(wchar_t const* str)
{
	int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str, -1, nullptr, 0, nullptr, nullptr);
	if (count == 0)
		return nullptr;
	String result(count - 1, 0);
	int ret = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str, -1, result.data(), count, nullptr, nullptr);
	return ret != 0 ? result : nullptr;
}

wchar_t const* StringUtils::Utf16Of(wchar_t* buffer, uint32_t size, char const* string)
{
	int ret = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, string, -1, buffer, size);
	return ret != 0 ? buffer : nullptr;
}

char const* StringUtils::Utf8Of(char* buffer, uint32_t size, wchar_t const* string)
{
	int ret = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string, -1, buffer, size, nullptr, nullptr);
	return ret != 0 ? buffer : nullptr;
}

wchar_t* StringUtils::Utf16OfAutoExpand(wchar_t* buffer, uint32_t size, char const* string)
{
	int count = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, string, -1, nullptr, 0);
	if (count <= size)
	{
		int ret = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, string, -1, buffer, size);
		return ret != 0 ? buffer : nullptr;
	}
	wchar_t* largerBuffer = Mem::Alloc<wchar_t>(count);
	int ret = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, string, -1, largerBuffer, count);
	if (ret != 0)
		return largerBuffer;
	Mem::Free(largerBuffer);
	return nullptr;
}

char* StringUtils::Utf8OfAutoExpand(char* buffer, uint32_t size, wchar_t const* string)
{
	int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string, -1, nullptr, 0, nullptr, nullptr);
	if (count <= size)
	{
		int ret = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string, -1, buffer, size, nullptr, nullptr);
		return ret != 0 ? buffer : nullptr;
	}
	char* largerBuffer = Mem::Alloc<char>(count);
	int ret = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, string, -1, largerBuffer, count, nullptr, nullptr);
	if (ret != 0)
		return largerBuffer;
	Mem::Free(largerBuffer);
	return nullptr;
}

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		VALUE PARSER
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
char const* StringUtils::Parse(char const* str, uint32_t& result)
{
	result = 0;
	for (; *str != 0 && isdigit(*str); str++)
		result = result * 10 + (*str - '0');
	return str;
}

uint32_t StringUtils::ToString(uint32_t value, char* buffer)
{
	uint32_t length = 0;
	do
	{
		buffer[length] = value % 10 + '0';
		value /= 10;
		length++;
	} while (value != 0);
	std::reverse(buffer, buffer + length);
	return length;
}

uint32_t StringUtils::ToString(uint32_t value, char* buffer, uint32_t size)
{
	uint32_t length = 0;
	do
	{
		if (length >= size)
			return 0;
		buffer[length] = value % 10 + '0';
		value /= 10;
		length++;
	} while (value != 0);
	std::reverse(buffer, buffer + length);
	return length;
}

static void ToStringInternal(uint32_t value, String& outAppend)
{
	uint32_t origLength = outAppend.length();
	do
	{
		outAppend.push_back(value % 10 + '0');
		value /= 10;
	} while (value != 0);
	std::reverse(outAppend.begin() + origLength, outAppend.end());
}

String StringUtils::ToString(float value, uint32_t precision)
{
	if (isnan(value))
		return "NaN";
	if (isinf(value))
		return value > 0.0f ? "+Inf" : "-Inf";
	String result;
	if (value < 0.0f)
	{
		result.push_back('-');
		value = -value;
	}
	uint32_t wholePart = value;
	ToStringInternal(wholePart, result);
	value -= wholePart;
	if (value < FLT_EPSILON)
		return result;
	result.push_back('.');
	for (uint32_t i = 0; i < precision; i++)
	{
		value *= 10.0f;
		wholePart = static_cast<uint32_t>(value);
		result.push_back(wholePart + '0');
		value -= wholePart;
		if (value < FLT_EPSILON)
			break;
	}
	return result;
}

String StringUtils::ToString(uint32_t value)
{
	String result;
	do
	{
		result.push_back(value % 10 + '0');
		value /= 10;
	} while (value != 0);
	std::reverse(result.begin(), result.end());
	return result;
}

String StringUtils::ToString(int32_t value)
{
	String result;
	if (value < 0)
	{
		result.push_back('-');
		value = -value;
	}
	ToStringInternal(static_cast<uint32_t>(value), result);
	return result;
}

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		BASIC OPERATION
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
char* StringUtils::Copy(char const* str)
{
	uint32_t length = strlen(str);
	char* buffer = Mem::Alloc<char>(length + 1);
	memcpy(buffer, str, length + 1);
	return buffer;
}

bool StringUtils::Append(char* dest, uint32_t size, char const* src)
{
	uint32_t destLength = strlen(dest);
	uint32_t srcLength = strlen(src);
	if (destLength + srcLength >= size)
		return false;
	memcpy(dest + destLength, src, srcLength);
	dest[destLength + srcLength] = 0;
	return true;
}

void StringUtils::Replace(char* buffer, char from, char to)
{
	while (*buffer != 0)
	{
		if (*buffer == from)
			*buffer = to;
		buffer++;
	}
}

/*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
		PATH MANIPULATION
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*/
char const* StringUtils::GetFileName(char const* fullPath)
{
	char const* result = fullPath;
	while (*fullPath != 0)
	{
		if (*fullPath == '\\' || *fullPath == '/')
			result = fullPath + 1;
		fullPath++;
	}
	return result;
}

char const* StringUtils::GetOuterPath(char const* path, char* buffer, uint32_t size)
{
	uint32_t i = 0;
	for (uint32_t j = 0; path[j] != 0; j++)
	{
		if (path[j] == '\\' || path[j] == '/')
			i = j;
	}
	if (i >= size)
		return nullptr;
	memcpy(buffer, path, i);
	buffer[i] = 0;
	return buffer;
}

void StringUtils::GetOuterPath(char* path)
{
	uint32_t i = 0;
	for (uint32_t j = 0; path[j] != 0; j++)
	{
		if (path[j] == '\\' || path[j] == '/')
			i = j;
	}
	path[i] = 0;
}

String StringUtils::JoinPath(StringView path, StringView folder)
{
	String result;
	result.resize(path.length() + folder.length() + 1);
	memcpy(result.data(), path.data(), path.length());
	result[path.length()] = '\\';
	memcpy(result.data() + path.length() + 1, folder.data(), folder.length());
	return result;
}

uint32_t StringUtils::JoinPath(char* path, uint32_t size, StringView folder)
{
	uint32_t lhsLen = strlen(path);
	uint32_t rhsLen = folder.length();
	if (lhsLen + rhsLen + 1 >= size)
		return 0;
	path[lhsLen] = '\\';
	memcpy(path + lhsLen + 1, folder.data(), rhsLen);
	path[lhsLen + rhsLen + 1] = 0;
	return lhsLen + rhsLen + 1;
}

uint32_t StringUtils::JoinPath(char* buffer, uint32_t size, StringView path, StringView folder)
{
	if (path.length() + folder.length() + 1 >= size)
		return 0;
	memcpy(buffer, path.data(), path.length());
	buffer[path.length()] = '\\';
	memcpy(buffer + path.length() + 1, folder.data(), folder.length());
	buffer[path.length() + folder.length() + 1] = 0;
	return path.length() + folder.length() + 1;
}