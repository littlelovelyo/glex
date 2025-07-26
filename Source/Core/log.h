#pragma once
#include "config.h"
#include "Platform/platform.h"
#include "Core/Container/function.h"
#include "Core/Utils/string.h"
#include <stdarg.h>

namespace glex
{
	enum class LogLevel : uint32_t
	{
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	enum class ConsoleColor : uint32_t
	{
		Ignore,
		Black,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,
		Gray,
		LightRed,
		LightGreen,
		LightYellow,
		LightBlue,
		LightMagenta,
		LightCyan,
		LightWhite
	};

	class Logger : private StaticClass
	{
	private:
		inline static Function<void(LogLevel, char const*)> s_logCallback;
		static uint32_t FormatSequence(char* buffer, ConsoleColor backgroundColor, ConsoleColor foregroundColor);

	public:
		template <typename Fn>
		static void RedirectLog(Fn&& fn)
		{
			s_logCallback = std::forward<Fn>(fn);
		}

		template <typename Obj>
		static void RedirectLog(Obj* obj, MemberFunctionPtr<Obj, void(LogLevel, char const*)> fn)
		{
			s_logCallback.BindMember(obj, fn);
		}

		template <LogLevel LEVEL>
		static void VLog(char const* format, va_list arglist)
		{
			char buffer[Limits::LOG_BUFFER_SIZE];
			char* actualBuffer;
			uint32_t pointer = 0;
			if (s_logCallback == nullptr)
			{
				ConsoleColor backgroundColor = ConsoleColor::Black;
				ConsoleColor foregroundColor = ConsoleColor::White;
				if constexpr (LEVEL == LogLevel::Debug)
					foregroundColor = ConsoleColor::LightBlue;
				if constexpr (LEVEL == LogLevel::Info)
					foregroundColor = ConsoleColor::Green;
				else if constexpr (LEVEL == LogLevel::Warning)
					foregroundColor = ConsoleColor::Yellow;
				else if constexpr (LEVEL == LogLevel::Error)
					foregroundColor = ConsoleColor::Red;
				else if constexpr (LEVEL == LogLevel::Fatal)
					backgroundColor = ConsoleColor::Red;
				pointer = FormatSequence(buffer, backgroundColor, foregroundColor);
				int32_t length = vsnprintf(buffer + pointer, Limits::LOG_BUFFER_SIZE - pointer, format, ap);
				if (length < 0)
					actualBuffer = nullptr;
				else if (length < Limits::LOG_BUFFER_SIZE - pointer)
					actualBuffer = buffer;
				else
				{
					actualBuffer = Mem::Alloc<char>(pointer + length + 1);
					memcpy(actualBuffer, buffer, pointer);
					length = vsnprintf(actualBuffer + pointer, length + 1, format, ap);
					if (length < 0)
					{
						Mem::Free(actualBuffer);
						actualBuffer = nullptr;
					}
				}
				if (actualBuffer != nullptr)
					puts(actualBuffer);
			}
			else
			{
				actualBuffer = StringUtils::FormatAutoExpand(buffer, format, arglist);
				if (actualBuffer != nullptr)
					s_logCallback(LEVEL, actualBuffer);
			}

			if constexpr (LEVEL == LogLevel::Fatal)
			{
				if (Platform::IsDebuggerPresent())
					Platform::DebugBreak();
				else
				{
					Platform::MessageBox(MessageBoxIcon::Error, "Fatal error", actualBuffer);
					Platform::Terminate();
				}
			}

			if (actualBuffer != buffer)
				Mem::Free(actualBuffer);
		}

		static void Trace(char const* format, ...);
		static void Debug(char const* format, ...);
		static void Info(char const* format, ...);
		static void Warn(char const* format, ...);
		static void Error(char const* format, ...);
		static void Fatal(char const* format, ...);
	};
}