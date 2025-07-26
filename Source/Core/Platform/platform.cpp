#include "Core/Platform/platform.h"
#include "Core/Utils/string.h"
#include "Core/Platform/window.h"
#include "Core/log.h"
#include "Core/Utils/raii.h"
#include "Core/Utils/temp_buffer.h"
#include "Core/Thread/thread.h"
#include <Core/Platform/shell.hpp>
#include <commdlg.h>
#include <shellapi.h>
using namespace glex;
#undef Yield

bool Platform::IsDebuggerPresent()
{
	return ::IsDebuggerPresent();
}

void Platform::Terminate()
{
	ExitProcess(EXIT_FAILURE);
}

#undef MessageBox
static UINT s_mbIcons[] = { 0, MB_ICONINFORMATION, MB_ICONWARNING, MB_ICONERROR, MB_ICONQUESTION };
void Platform::MessageBox(MessageBoxIcon icon, char const* title, char const* message)
{
	wchar_t titleBuffer[Limits::NAME_LENGTH + 1];
	wchar_t* wideTitle = StringUtils::Utf16OfAutoExpand(titleBuffer, title);
	Nullable<WideString> wideMessage = StringUtils::Utf16Of(message);
	MessageBoxW(reinterpret_cast<HWND>(Window::GetWin32Handle()), wideMessage != nullptr ? wideMessage->c_str() : nullptr, titleBuffer, s_mbIcons[*icon]);
	if (wideTitle != titleBuffer)
		Mem::Free(wideTitle);
}

bool Platform::ConfirmMessageBox(MessageBoxIcon icon, char const* title, char const* message)
{
	wchar_t titleBuffer[Limits::NAME_LENGTH + 1];
	wchar_t* wideTitle = StringUtils::Utf16OfAutoExpand(titleBuffer, title);
	Nullable<WideString> wideMessage = StringUtils::Utf16Of(message);
	int ret = MessageBoxW(reinterpret_cast<HWND>(Window::GetWin32Handle()), wideMessage != nullptr ? wideMessage->c_str() : nullptr, titleBuffer, s_mbIcons[*icon] | MB_YESNO);
	if (wideTitle != titleBuffer)
		Mem::Free(wideTitle);
	return ret == IDYES;
}

Nullable<String> Platform::GetWorkingDirectory()
{
	wchar_t buffer[Limits::PATH_LENGTH + 1];
	uint32_t ret = GetCurrentDirectoryW(Limits::PATH_LENGTH + 1, buffer);
	if (ret == 0 || ret >= Limits::PATH_LENGTH + 1)
		return nullptr;
	return StringUtils::Utf8Of(buffer);
}

bool Platform::SetWorkingDirectory(char const* dir)
{
	wchar_t buffer[Limits::PATH_LENGTH + 1];
	return SetCurrentDirectoryW(StringUtils::Utf16Of(buffer, dir));
}

// Title is not nullable.
Nullable<String> Platform::OpenDirectoryDialog()
{
	// This is the file dialog.
	IFileDialog* pfd;
	if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		return nullptr;
	AutoCleaner pfdCleaner([&]() { pfd->Release(); });

	// This is the event listener.
	IFileDialogEvents* pfde;
	if (!SUCCEEDED(CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde))))
		return nullptr;
	AutoCleaner pfdeCleaner([&]() { pfde->Release(); });

	// Assigns event listener.
	DWORD dwCookie;
	if (!SUCCEEDED(pfd->Advise(pfde, &dwCookie)))
		return nullptr;
	AutoCleaner adviseCleaner([&]() { pfd->Unadvise(dwCookie); });

	// Set options.
	DWORD dwFlags;
	if (!SUCCEEDED(pfd->GetOptions(&dwFlags)))
		return nullptr;
	dwFlags |= FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS;
	if (!SUCCEEDED(pfd->SetOptions(dwFlags)))
		return nullptr;

	// Show the dialog.
	if (!SUCCEEDED(pfd->Show(reinterpret_cast<HWND>(Window::GetWin32Handle()))))
		return nullptr;

	// Get the result.
	IShellItem* psiResult;
	if (!SUCCEEDED(pfd->GetResult(&psiResult)))
		return nullptr;
	AutoCleaner resultCleaner([&]() { psiResult->Release(); });

	PWSTR pszFilePath;
	if (!SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
		return nullptr;
	AutoCleaner nameCleaner([&]() { CoTaskMemFree(pszFilePath); });

	return StringUtils::Utf8Of(pszFilePath);
}

Nullable<String> Platform::OpenFileDialog(char const* filter)
{
	wchar_t utf16Filter[Limits::PATH_LENGTH + 1];
	wchar_t outFileName[Limits::PATH_LENGTH + 1];

	OPENFILENAMEW openFile;
	openFile.lStructSize = sizeof(OPENFILENAMEW);
	openFile.hwndOwner = reinterpret_cast<HWND>(Window::GetWin32Handle());

	if (filter == nullptr)
		openFile.lpstrFilter = nullptr;
	else
	{
		// Filter is NULL-seperated. Change it to '|' to make things simple.
		if (StringUtils::Utf16Of(utf16Filter, filter) == nullptr)
			return nullptr;
		for (uint32_t i = 0; i < Limits::PATH_LENGTH; i++)
		{
			if (utf16Filter[i] == '|')
				utf16Filter[i] = 0;
			else if (utf16Filter[i] == 0)
			{
				utf16Filter[i + 1] = 0;
				goto FILTER_SUCCESS;
			}
		}
		return nullptr;
	FILTER_SUCCESS:
		openFile.lpstrFilter = utf16Filter;
	}

	openFile.lpstrCustomFilter = nullptr;
	openFile.nFilterIndex = 0;

	outFileName[0] = 0;
	openFile.lpstrFile = outFileName;
	openFile.nMaxFile = Limits::PATH_LENGTH + 1;

	openFile.lpstrFileTitle = nullptr;

	openFile.lpstrInitialDir = nullptr;
	openFile.lpstrTitle = nullptr;
	openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	openFile.lpstrDefExt = nullptr;
	openFile.lCustData = 0;

	if (!GetOpenFileNameW(&openFile))
		return nullptr;
	return StringUtils::Utf8Of(outFileName);
}

Nullable<String> Platform::SaveFileDialog(char const* filter)
{
	wchar_t utf16Filter[Limits::PATH_LENGTH + 1];
	wchar_t outFileName[Limits::PATH_LENGTH + 1];

	OPENFILENAMEW openFile;
	openFile.lStructSize = sizeof(OPENFILENAMEW);
	openFile.hwndOwner = reinterpret_cast<HWND>(Window::GetWin32Handle());

	if (filter == nullptr)
		openFile.lpstrFilter = nullptr;
	else
	{
		// Filter is NULL-seperated. Change it to '|' to make things simple.
		if (StringUtils::Utf16Of(utf16Filter, filter) == nullptr)
			return nullptr;
		for (uint32_t i = 0; i < Limits::PATH_LENGTH; i++)
		{
			if (utf16Filter[i] == '|')
				utf16Filter[i] = 0;
			else if (utf16Filter[i] == 0)
			{
				utf16Filter[i + 1] = 0;
				goto FILTER_SUCCESS;
			}
		}
		return nullptr;
	FILTER_SUCCESS:
		openFile.lpstrFilter = utf16Filter;
	}

	openFile.lpstrCustomFilter = nullptr;
	openFile.nFilterIndex = 0;

	outFileName[0] = 0;
	openFile.lpstrFile = outFileName;
	openFile.nMaxFile = Limits::PATH_LENGTH + 1;

	openFile.lpstrFileTitle = nullptr;

	openFile.lpstrInitialDir = nullptr;
	openFile.lpstrTitle = nullptr;
	openFile.Flags = 0;
	openFile.lpstrDefExt = nullptr;
	openFile.lCustData = 0;

	if (!GetSaveFileNameW(&openFile))
		return nullptr;
	return StringUtils::Utf8Of(outFileName);
}

bool Platform::OpenFile(char const* file)
{
	static bool s_comInitialized = false;
	if (!s_comInitialized)
	{
		if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) != S_OK)
			return false;
		s_comInitialized = true;
	}
	wchar_t wpath[Limits::PATH_LENGTH + 1];
	if (StringUtils::Utf16Of(wpath, file) == nullptr)
		return false;
	return reinterpret_cast<uint64_t>(ShellExecuteW(NULL, L"open", wpath, nullptr, nullptr, SW_SHOW)) > 32;
}

#undef DeleteFile
bool Platform::DeleteFile(char const* file, bool moveToRecycleBin)
{
	wchar_t path[Limits::PATH_LENGTH + 2];
	if (StringUtils::Utf16Of(path, Limits::PATH_LENGTH + 1, file) == nullptr)
		return false;
	path[wcslen(path) + 1] = 0;

	SHFILEOPSTRUCTW fst;
	fst.hwnd = NULL;
	fst.wFunc = FO_DELETE;
	fst.pFrom = path;
	fst.pTo = nullptr;
	fst.fFlags = moveToRecycleBin ? FOF_ALLOWUNDO : 0;
	fst.fAnyOperationsAborted = FALSE;
	fst.hNameMappings = nullptr;
	fst.lpszProgressTitle = nullptr;
	return SHFileOperationW(&fst) == 0;
}

#undef MoveFile
bool Platform::MoveFile(char const* from, char const* to)
{
	wchar_t wfrom[Limits::PATH_LENGTH + 2];
	wchar_t wto[Limits::PATH_LENGTH + 2];
	if (StringUtils::Utf16Of(wfrom, Limits::PATH_LENGTH + 1, from) == nullptr || StringUtils::Utf16Of(wto, Limits::PATH_LENGTH + 1, to) == nullptr)
		return false;
	wfrom[wcslen(wfrom) + 1] = 0;
	wto[wcslen(wto) + 1] = 0;

	SHFILEOPSTRUCTW fst;
	fst.hwnd = NULL;
	fst.wFunc = FO_MOVE;
	fst.pFrom = wfrom;
	fst.pTo = wto;
	fst.fFlags = 0;
	fst.fAnyOperationsAborted = FALSE;
	fst.hNameMappings = nullptr;
	fst.lpszProgressTitle = nullptr;
	return SHFileOperationW(&fst) == 0;
}

#undef CopyFile
bool Platform::CopyFile(char const* from, char const* to)
{
	wchar_t wfrom[Limits::PATH_LENGTH + 2];
	wchar_t wto[Limits::PATH_LENGTH + 2];
	if (StringUtils::Utf16Of(wfrom, Limits::PATH_LENGTH + 1, from) == nullptr || StringUtils::Utf16Of(wto, Limits::PATH_LENGTH + 1, to) == nullptr)
		return false;
	wfrom[wcslen(wfrom) + 1] = 0;
	wto[wcslen(wto) + 1] = 0;

	SHFILEOPSTRUCTW fst;
	fst.hwnd = NULL;
	fst.wFunc = FO_COPY;
	fst.pFrom = wfrom;
	fst.pTo = wto;
	fst.fFlags = 0;
	fst.fAnyOperationsAborted = FALSE;
	fst.hNameMappings = nullptr;
	fst.lpszProgressTitle = nullptr;
	return SHFileOperationW(&fst) == 0;
}

bool Platform::EnumerateDirectory(char const* path, Vector<String>& outDirectories, Vector<String>& outFiles)
{
	outDirectories.clear();
	outFiles.clear();
	wchar_t utf16Path[Limits::PATH_LENGTH + 1];
	if (StringUtils::Utf16Of(utf16Path, Limits::PATH_LENGTH - 1, path) == nullptr)
		return false;
	uint32_t pathLength = wcslen(utf16Path);
	utf16Path[pathLength] = '\\';
	utf16Path[pathLength + 1] = '*';
	utf16Path[pathLength + 2] = 0;

	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(utf16Path, &data);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	AutoCleaner close([&]() { FindClose(h); });
	do
	{
		char utf8Name[Limits::PATH_LENGTH + 1];
		if (StringUtils::Utf8Of(utf8Name, data.cFileName) == nullptr)
			return false;
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(utf8Name, ".") == 0 || strcmp(utf8Name, "..") == 0)
				continue;
			outDirectories.emplace_back(utf8Name);
		}
		else
			outFiles.emplace_back(utf8Name);
	} while (FindNextFileW(h, &data));
	return GetLastError() == ERROR_NO_MORE_FILES;
}

#undef CreateDirectory
bool Platform::CreateDirectory(char const* path)
{
	wchar_t wpath[Limits::PATH_LENGTH + 1];
	if (StringUtils::Utf16Of(wpath, path) == nullptr)
		return false;
	return CreateDirectoryW(wpath, nullptr);
}

Nullable<bool> Platform::DirectoryExists(char const* path)
{
	wchar_t wpath[Limits::PATH_LENGTH + 1];
	if (StringUtils::Utf16Of(wpath, path) == nullptr)
		return nullptr;
	DWORD result = GetFileAttributesW(wpath);
	if (result != INVALID_FILE_ATTRIBUTES)
		return result & FILE_ATTRIBUTE_DIRECTORY;
	return nullptr;
}

Nullable<bool> Platform::FileExists(char const* file)
{
	wchar_t path[Limits::PATH_LENGTH + 1];
	if (StringUtils::Utf16Of(path, file) == nullptr)
		return nullptr;
	DWORD result = GetFileAttributesW(path);
	if (result != INVALID_FILE_ATTRIBUTES)
		return !(result & FILE_ATTRIBUTE_DIRECTORY);
	return nullptr;
}

std::pair<Nullable<bool>, String> Platform::RunCommandLine(char const* commandLine)
{
	SECURITY_ATTRIBUTES secAttrib;
	secAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttrib.lpSecurityDescriptor = nullptr;
	secAttrib.bInheritHandle = TRUE;

	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &secAttrib, 0))
		return { nullptr, "" };
	AutoCleaner pipeCleaner([&]()
	{
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
	});
	if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0))
		return { nullptr, "" };

	wchar_t wideArg[Limits::PATH_LENGTH + 1];
	wchar_t* wideCommandLine = StringUtils::Utf16OfAutoExpand(wideArg, commandLine);
	if (wideCommandLine == nullptr)
		return { nullptr, "" };
	AutoCleaner cmdCleaner([&]()
	{
		if (wideCommandLine != wideArg)
			Mem::Free(wideCommandLine);
	});

	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdOutput = hWritePipe;
	startupInfo.hStdError = hWritePipe;
	PROCESS_INFORMATION processInfo;
	if (!CreateProcessW(nullptr, wideCommandLine, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
		return { nullptr, "" };
	AutoCleaner processCleaner([&]()
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	});

	String string;
	char buffer[1024];
	for (;;)
	{
		DWORD totalBytesAvailable;
		if (!PeekNamedPipe(hReadPipe, nullptr, 0, nullptr, &totalBytesAvailable, nullptr))
			return { nullptr, "" };

		if (totalBytesAvailable != 0)
		{
			uint32_t bytesToRead = glm::min<uint32_t>(1024, totalBytesAvailable);
			DWORD bytesRead;
			if (!ReadFile(hReadPipe, buffer, bytesToRead, &bytesRead, nullptr) || bytesRead != bytesToRead)
				return { nullptr, "" };
			string.append(buffer, buffer + bytesToRead);
		}

		DWORD ret = WaitForSingleObject(processInfo.hProcess, 0); // How can it fail?
		if (ret == WAIT_TIMEOUT)
			Thread::Yield();
		else if (ret == WAIT_OBJECT_0)
			break;
		else
			return { nullptr, "" };
	}
	string.push_back(0);

	// The default encoding is ACP.

	uint32_t codePage = GetACP();
	if (codePage != CP_UTF8)
	{
		Logger::Warn("You're not using UTF-8 code page. I assume you're using ANSI. But it's strongly recommended you use UTF-8.");

		// ASCII Compatibility check. we SIMPLY ASSUME it's ASCII compatible.
		for (char c : string)
		{
			if (!StringUtils::IsASCII(c))
				goto NEEDS_CONVERSION;

		}
		goto ASCII_COMPATIBLE;

	NEEDS_CONVERSION:
		uint32_t utf16Size = MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length() + 1, nullptr, 0);
		if (utf16Size == 0)
			return { nullptr, "" };
		TemporaryBuffer<wchar_t> buffer = Mem::Alloc<wchar_t>(utf16Size);
		if (MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length() + 1, buffer.Get(), utf16Size) == 0)
			return { nullptr, "" };
		uint32_t utf8Size = WideCharToMultiByte(CP_UTF8, 0, buffer.Get(), utf16Size, nullptr, 0, nullptr, nullptr);
		if (utf8Size == 0)
			return { nullptr, "" };
		string.resize(utf8Size - 1);
		if (WideCharToMultiByte(CP_UTF8, 0, buffer.Get(), utf16Size, string.data(), utf8Size, nullptr, nullptr) == 0)
			return { nullptr, "" };
	}

ASCII_COMPATIBLE:
	DWORD exitCode;
	if (!GetExitCodeProcess(processInfo.hProcess, &exitCode))
		return { nullptr, "" };
	return { exitCode == 0, string };
}

uint32_t Platform::GetProcessorCount()
{
	return GetCurrentProcessorNumber();
}

float Platform::GetDoubleClickTime()
{
	return ::GetDoubleClickTime();
}