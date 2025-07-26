/* #include "Core/gassert.h"
#include "Core/Utils/gstring.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")
#include <stdio.h>
#include <Windows.h>

void glex::Abort(char const* reason, ...)
{
	char stringBuffer[Limits::STRING_BUFFER_SIZE];
	va_list vp;
	va_start(vp, reason);
	vsnprintf(stringBuffer, Limits::STRING_BUFFER_SIZE, reason, vp);
	va_end(vp);

#if GLEX_TEST_FEATURES
	printf(GLEX_LOG_RED("%s\n"), stringBuffer);
	if (IsDebuggerPresent())
		__debugbreak();
	else
	{
		system("pause");
		ExitProcess(0);
	}
#else
	MessageBoxW(NULL, Utf16Of(stringBuffer), L"Abort", MB_OK | MB_ICONERROR);
	ExitProcess(0);
#endif
}

void glex::Warn(char const* warning, ...)
{
	char stringBuffer[Limits::STRING_BUFFER_SIZE];
	va_list vp;
	va_start(vp, warning);
	vsnprintf(stringBuffer, Limits::STRING_BUFFER_SIZE, warning, vp);
	va_end(vp);

#if GLEX_TEST_FEATURES
	printf(GLEX_LOG_YELLOW("%s\n"), stringBuffer);
#else
	MessageBoxW(NULL, Utf16Of(stringBuffer), L"Warning", MB_OK | MB_ICONWARNING);
#endif
}

void glex::Error(char const* error, ...)
{
	char stringBuffer[Limits::STRING_BUFFER_SIZE];
	va_list vp;
	va_start(vp, error);
	vsnprintf(stringBuffer, Limits::STRING_BUFFER_SIZE, error, vp);
	va_end(vp);

#if GLEX_TEST_FEATURES
	printf(GLEX_LOG_RED("%s\n"), stringBuffer);
#else
	MessageBoxW(NULL, Utf16Of(stringBuffer), L"Error", MB_OK | MB_ICONERROR);
#endif
} */