#include "fileasync.h"
#include "Memory/mem.h"

using namespace glex;

AsyncFileReader::AsyncFileReader(pathchr_t const* path, uint32_t bufferSize, FileBuffering buffering)
{
	m_handle = CreateFileW(path, FILE_GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, static_cast<uint32_t>(buffering), NULL);
	if (m_handle == INVALID_HANDLE_VALUE)
		return;
	m_fileSize = GetFileSize(m_handle, nullptr);
	if (m_fileSize == INVALID_FILE_SIZE)
		goto fail_on_first_allocation;
	m_buffers[0] = static_cast<uint8_t*>(Mem::Alloc(bufferSize));
	if (m_buffers[0] == nullptr)
		goto fail_on_first_allocation;
	m_buffers[1] = static_cast<uint8_t*>(Mem::Alloc(bufferSize));
	if (m_buffers[1] == nullptr)
		goto fail_on_second_allocation;
	memset(&m_overlapped, 0, sizeof(OVERLAPPED));
	m_bufferSize = bufferSize;
	m_bufferPointer = 0;
	m_filePointer = 0;
	m_frontBuffer = 0;
	m_backBuffer = 1;
	m_hitEOF = false;
	m_realEOF = false;
	ReadAsync(m_frontBuffer);
	return;
fail_on_second_allocation:
	Mem::Free(m_buffers[0]);
fail_on_first_allocation:
	CloseHandle(m_handle);
	m_handle = INVALID_HANDLE_VALUE;
	m_buffers[0] = nullptr;
	m_buffers[1] = nullptr;
}

AsyncFileReader::~AsyncFileReader()
{
	CloseHandle(m_handle);
	Mem::Free(m_buffers[0]); // mi_free a nullptr should be OK.
	Mem::Free(m_buffers[1]);
}

AsyncFileReader::AsyncFileReader(AsyncFileReader&& rhs) : m_handle(rhs.m_handle), m_overlapped(rhs.m_overlapped),
m_buffers { rhs.m_buffers[0], rhs.m_buffers[1] }, m_bufferSize(rhs.m_bufferSize), m_fileSize(rhs.m_fileSize),
m_filePointer(rhs.m_filePointer), m_bufferPointer(rhs.m_bufferPointer),
m_frontBuffer(rhs.m_frontBuffer), m_backBuffer(rhs.m_backBuffer), m_readingBuffer(rhs.m_readingBuffer),
m_hitEOF(rhs.m_hitEOF), m_reading(rhs.m_reading), m_realEOF(rhs.m_realEOF)
{
	rhs.m_handle = INVALID_HANDLE_VALUE;
	rhs.m_buffers[0] = nullptr;
	rhs.m_buffers[1] = nullptr;
}

void AsyncFileReader::ReadAsync(uint8_t bufferIndex)
{
	if (m_hitEOF)
		return;
	m_reading = true;
	m_readingBuffer = bufferIndex;
	DWORD actualRead;
	// ReadFile may return TRUE if the operation is done synchronously.
	if (ReadFile(m_handle, m_buffers[bufferIndex], m_bufferSize, &actualRead, &m_overlapped))
		OnReadFinished(actualRead);
	else if (GetLastError() != ERROR_IO_PENDING)
	{
		m_reading = false;
		m_hitEOF = true;
	}
}

void AsyncFileReader::OnReadFinished(uint32_t bytesRead)
{
	m_reading = false;
	m_overlapped.Offset += bytesRead;
	if (bytesRead < m_bufferSize)
		m_hitEOF = true;
}

void AsyncFileReader::Wait()
{
	if (!m_reading)
		return;
	DWORD bytesRead;
	if (GetOverlappedResult(m_handle, &m_overlapped, &bytesRead, TRUE))
		OnReadFinished(bytesRead);
	else
	{
		m_hitEOF = true;
		m_reading = false;
	}
}

void AsyncFileReader::Seek(int32_t move, FilePosition from)
{
	uint32_t filePointer;
	switch (from)
	{
		case FilePosition::Begin: filePointer = move; break;
		case FilePosition::End: filePointer = m_fileSize - move; break;
		default: filePointer = m_filePointer + move;
	}
	if (filePointer > m_fileSize)
		filePointer = m_fileSize;
	if (filePointer == m_filePointer)
		return;
	Wait();
	if (filePointer > m_filePointer)
	{
		uint32_t dp = filePointer - m_filePointer;
		if (dp < m_bufferSize - m_bufferPointer)
		{
			m_bufferPointer += dp;
			return;
		}
	}
	else
	{
		uint32_t dp = m_filePointer - filePointer;
		if (m_bufferPointer >= dp)
		{
			m_bufferPointer -= dp;
			return;
		}
	}
	m_filePointer = filePointer;
	m_hitEOF = filePointer == m_fileSize;
	m_realEOF = m_hitEOF;
	m_overlapped.Offset = filePointer;
	m_bufferPointer = 0;
	ReadAsync(m_frontBuffer);
}

uint32_t AsyncFileReader::Read(void* buffer, uint32_t read)
{
	if (m_realEOF)
		return 0;
	if (m_readingBuffer == m_frontBuffer)
	{
		Wait();
		ReadAsync(m_backBuffer);
	}
	uint32_t bytesRead = 0;
	while (read > 0)
	{
		uint32_t copy = std::min<uint32_t>(m_overlapped.Offset - m_filePointer, std::min(m_bufferSize - m_bufferPointer, read));
		memcpy(buffer, m_buffers[m_frontBuffer] + m_bufferPointer, copy);
		read -= copy;
		bytesRead += copy;
		m_filePointer += copy;
		m_bufferPointer += copy;
		if (m_filePointer == m_overlapped.Offset)
		{
			m_realEOF = true;
			break; // Reached EOF.
		}
		if (m_bufferPointer == m_bufferSize)
		{
			Wait();
			m_frontBuffer ^= 1;
			m_backBuffer ^= 1;
			m_bufferPointer = 0;
			ReadAsync(m_backBuffer);
		}
	}
	return bytesRead;
}