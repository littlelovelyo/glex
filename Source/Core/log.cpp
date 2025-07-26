#include "Core/log.h"
#include "Utils/string.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")

using namespace glex;

uint32_t Logger::FormatSequence(char* buffer, ConsoleColor backgroundColor, ConsoleColor foregroundColor)
{
	uint32_t pointer = 0;
	bool special = backgroundColor != ConsoleColor::Ignore || foregroundColor != ConsoleColor::Ignore;
	if (special)
	{
		buffer[0] = '\x1b';
		buffer[1] = '[';
		pointer = 2;
		if (backgroundColor != ConsoleColor::Ignore)
			pointer += StringUtils::ToString(backgroundColor > ConsoleColor::White ? *backgroundColor + 91 : *backgroundColor + 39, buffer + 2);
		if (foregroundColor != ConsoleColor::Ignore)
		{
			if (pointer != 2)
				buffer[pointer++] = ';';
			pointer += StringUtils::ToString(foregroundColor > ConsoleColor::White ? *foregroundColor + 81 : *foregroundColor + 29, buffer + pointer);
		}
		buffer[pointer++] = 'm';
	}
	return pointer;
}

void Logger::Trace(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Trace>(format, arglist);
	va_end(arglist);
}

void Logger::Debug(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Debug>(format, arglist);
	va_end(arglist);
}

void Logger::Info(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Info>(format, arglist);
	va_end(arglist);
}

void Logger::Warn(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Warning>(format, arglist);
	va_end(arglist);
}

void Logger::Error(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Error>(format, arglist);
	va_end(arglist);
}

void Logger::Fatal(char const* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	VLog<LogLevel::Fatal>(format, arglist);
	va_end(arglist);
}