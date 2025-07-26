#pragma once
#include "file.h"
#include "Core/commdefs.h"
#include "Core/Container/basic.h"
#include "Core/Utils/temp_buffer.h"

namespace glex
{
	class FileSync
	{
	private:
		uint64_t m_handle;
		uint64_t m_fileSize;

	public:
		static std::pair<TemporaryBuffer<void>, uint64_t> ReadAllContent(char const* path, uint32_t alignment = 0);

		FileSync(char const* path, FileAccess access, FileOpen openMode, FileFlags flags = FileFlags::Default);
		~FileSync();
		FileSync(FileSync&& rhs);
		FileSync& operator=(FileSync&& rhs);
		bool operator==(nullptr_t rhs) const { return m_handle == -1; }
		uint64_t Size() const { return m_fileSize; }
		bool Seek(int64_t move, FilePosition from);
		uint32_t Read(void* buffer, uint32_t read);
		uint32_t ReadString(char* buffer, uint32_t maxLength);
		bool ReadString(uint32_t maxLength, String& outStirng);
		uint32_t Write(void const* data, uint32_t size);
		bool WriteString(StringView string);

		/* uint32_t ReadOverlapped(uint32_t& offset, void* buffer, uint32_t size);
		uint32_t ReadStringOverlapped(uint32_t& offset, char* buffer, uint32_t maxLength); */

		template <typename T>
		bool Read(T& value)
		{
			return Read(&value, sizeof(T)) == sizeof(T);
		}

		template <typename T>
		bool Write(T const& value)
		{
			return Write(&value, sizeof(T)) == sizeof(T);
		}

		/* template <typename T>
		bool ReadOverlapped(uint32_t& offset, T& value)
		{
			return ReadOverlapped(offset, &value, sizeof(T)) == sizeof(T);
		} */
	};
}