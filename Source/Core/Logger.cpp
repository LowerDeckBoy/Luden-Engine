#include "Logger.hpp"
#include <chrono>
#include <print>

namespace Luden::Core
{
	static std::string LogLevelToString(LogLevel Severity)
	{
		switch (Severity)
		{
		case LogLevel::Info:	return "\x1b[37m[Info]";
		case LogLevel::Warning:	return "\x1b[33m[Warning]";
		case LogLevel::Error:	return "\x1b[31m[Error]";
		case LogLevel::Fatal:	return "\x1b[31m[Fatal]";
		case LogLevel::Debug:	return "\x1b[32m[Debug]";
		default:				return "\x1b[37m[Info]";
		}
	}

	static std::string QueryLogTime()
	{
		return std::format("[{:%X}]", std::chrono::system_clock::now());
	}

	void Logger::WriteLog(LogLevel Level, std::string_view Message)
	{
		std::println("{0} {1} {2}\x1b[37m", QueryLogTime(), LogLevelToString(Level), Message);
	}

	void Logger::WriteLog(LogLevel Level, std::wstring_view Message)
	{
		std::println("{0} {1} {2}\x1b[37m", QueryLogTime(), LogLevelToString(Level), String::FromWide(Message));
	}
} // namespace Luden::Core
