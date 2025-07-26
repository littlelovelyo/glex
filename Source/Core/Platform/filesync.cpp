#include "Core/Platform/filesync.h"
#include "Core/Utils/string.h"
#include "Core/Thread/event.h"
#include "Core/log.h"
#include "config.h"
#include <Windows.h>

using namespace glex;

std::pair<TemporaryBuffer<void>, uint64_t> FileSync::ReadAllContent(char const* path, uint32_t alignment)
{
	FileSync file(path, FileAccess::Read, FileOpen::OpenExisting);
	if (file == nullptr)
		return { nullptr, 0 };
	void* buffer = Mem::Alloc(file.Size(), alignment);
	if (file.Read(buffer, file.Size()) == file.Size())
		return { buffer, file.Size() };
	Mem::Free(buffer);
	return { nullptr, 0 };
}

FileSync::FileSync(char const* path, FileAccess access, FileOpen openMode, FileFlags flags)
{
	DWORD desiredAccess, shareMode;
	switch (access)
	{
		case FileAccess::Write: desiredAccess = GENERIC_WRITE; shareMode = NULL; break;
		case FileAccess::ReadWrite: desiredAccess = GENERIC_READ | GENERIC_WRITE; shareMode = NULL; break;
		default: desiredAccess = GENERIC_READ; shareMode = FILE_SHARE_READ; break;
	}
	HANDLE& handle = reinterpret_cast<HANDLE&>(m_handle);
	wchar_t pathBuffer[Limits::PATH_LENGTH + 1];
	handle = CreateFileW(StringUtils::Utf16Of(pathBuffer, path), desiredAccess, shareMode, nullptr, static_cast<uint32_t>(openMode), static_cast<uint32_t>(flags), nullptr);
	if (handle == INVALID_HANDLE_VALUE)
		return;
	if (!GetFileSizeEx(handle, reinterpret_cast<LARGE_INTEGER*>(&m_fileSize)))
	{
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
}

FileSync::~FileSync()
{
	CloseHandle(reinterpret_cast<HANDLE>(m_handle));
}

FileSync::FileSync(FileSync&& rhs) : m_handle(rhs.m_handle), m_fileSize(rhs.m_fileSize)
{
	rhs.m_handle = -1;
}

FileSync& FileSync::operator=(FileSync&& rhs)
{
	std::swap(m_handle, rhs.m_handle);
	m_fileSize = rhs.m_fileSize;
}

bool FileSync::Seek(int64_t move, FilePosition from)
{
	return SetFilePointerEx(reinterpret_cast<HANDLE>(m_handle), static_cast<LARGE_INTEGER>(move), nullptr, static_cast<uint32_t>(from));
}

uint32_t FileSync::Read(void* buffer, uint32_t read)
{
	DWORD actualRead;
	ReadFile(reinterpret_cast<HANDLE>(m_handle), buffer, read, &actualRead, nullptr);
	return actualRead;
}

uint32_t FileSync::ReadString(char* buffer, uint32_t maxLength)
{
	uint32_t length;
	if (!Read(length) || length > maxLength)
		return -1;
	if (Read(buffer, length) != length)
		return -1;
	buffer[length] = 0;
	if (!StringUtils::IsValidUtf8(buffer))
		return -1;
	return length;
}

bool FileSync::ReadString(uint32_t maxLength, String& outString)
{
	uint32_t length;
	if (!Read(length) || length > maxLength)
		return false;
	outString.resize(length);
	return Read(outString.data(), length) == length && StringUtils::IsValidUtf8(outString.c_str());
}

/* uint32_t FileSync::ReadOverlapped(uint32_t& offset, void* buffer, uint32_t size)
{
	DWORD read;
	OVERLAPPED overlapped = {};
	overlapped.Offset = offset;
	overlapped.OffsetHigh = 0;
	Event* event = Event::Get(true);
	overlapped.hEvent = reinterpret_cast<HANDLE>(event->Handle());
	if (ReadFile(reinterpret_cast<HANDLE>(m_handle), buffer, size, &read, &overlapped))
	{
		offset += read;
	}
	else if (GetLastError() == ERROR_IO_PENDING)
	{
		GetOverlappedResult(reinterpret_cast<HANDLE>(m_handle), &overlapped, &read, TRUE);
		offset += read;
	}
	Event::Release(event);
	return read;
}

uint32_t FileSync::ReadStringOverlapped(uint32_t& offset, char* buffer, uint32_t maxLength)
{
	uint32_t length;
	if (!ReadOverlapped(offset, length) || length > maxLength)
		return -1;
	if (ReadOverlapped(offset, buffer, length) != length)
		return -1;
	buffer[length] = 0;
	return length;
} */

uint32_t FileSync::Write(void const* data, uint32_t size)
{
	DWORD written;
	WriteFile(reinterpret_cast<HANDLE>(m_handle), data, size, &written, nullptr);
	return written;
}

bool FileSync::WriteString(StringView string)
{
	return Write(string.length()) && Write(string.data(), string.length()) == string.length();
}