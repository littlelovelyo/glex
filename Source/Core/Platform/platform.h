#pragma once
#include "Core/commdefs.h"
#include "Core/Container/basic.h"
#include "Core/Container/nullable.h"

namespace glex
{
	enum class MessageBoxIcon : uint32_t
	{
		None,
		Info,
		Warning,
		Error,
		Question
	};

	// Use UniquePtr<char const> to represent nullable string.
	class Platform : private StaticClass
	{
	public:
		static void DebugBreak() { __debugbreak(); }
		static bool IsDebuggerPresent();
		static void Terminate();
		static void MessageBox(MessageBoxIcon icon, char const* title, char const* message);
		static bool ConfirmMessageBox(MessageBoxIcon icon, char const* title, char const* message);
		static Nullable<String> GetWorkingDirectory();
		static bool SetWorkingDirectory(char const* dir);
		static Nullable<String> OpenDirectoryDialog();
		static Nullable<String> OpenFileDialog(char const* filter);
		static Nullable<String> SaveFileDialog(char const* filter);
		static bool OpenFile(char const* file); // Open in shell.
		static bool DeleteFile(char const* file, bool moveToRecycleBin = true);
		static bool MoveFile(char const* from, char const* to);
		static bool CopyFile(char const* from, char const* to);
		static bool EnumerateDirectory(char const* path, Vector<String>& outDirectories, Vector<String>& outFiles);
		static bool CreateDirectory(char const* path); // Only one folder can be created.
		static Nullable<bool> DirectoryExists(char const* path);
		static Nullable<bool> FileExists(char const* file);
		static std::pair<Nullable<bool>, String> RunCommandLine(char const* commandLine);
		static uint32_t GetProcessorCount();
		static float GetDoubleClickTime();
	};
}