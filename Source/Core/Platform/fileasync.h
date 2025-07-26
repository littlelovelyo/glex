#pragma once
#include "file.h"
#include "platform.h"

namespace glex
{

class AsyncFileReader
{
private:
	HANDLE m_handle;
	OVERLAPPED m_overlapped;
	uint8_t* m_buffers[2];
	uint32_t m_bufferSize, m_fileSize, m_filePointer, m_bufferPointer;
	uint8_t m_frontBuffer, m_backBuffer, m_readingBuffer;
	bool m_hitEOF, m_reading, m_realEOF;

	void ReadAsync(uint8_t bufferIndex);
	void OnReadFinished(uint32_t bytesRead);
	void Wait();

public:
	AsyncFileReader(pathchr_t const* path, uint32_t bufferSize = 65536, FileBuffering buffering = FileBuffering::None);
	~AsyncFileReader();
	AsyncFileReader(AsyncFileReader&& rhs);
	bool Valid() const { return m_handle != INVALID_HANDLE_VALUE; }
	uint32_t Size() const { return m_fileSize; }
	bool HitEOF() const { return m_realEOF; }
	void Seek(int32_t move, FilePosition from);
	uint32_t Read(void* buffer, uint32_t read);
};

}