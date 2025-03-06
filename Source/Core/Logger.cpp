#include "Logger.hpp"
#include <chrono>

namespace Luden::Core
{
	static std::string LogLevelToString(LogLevel Severity)
	{
		switch (Severity)
		{
		case LogLevel::Info:	return "\x1b[37m[Info]\x1b[37m";
		case LogLevel::Warning:	return "\x1b[33m[Warning]\x1b[37m";
		case LogLevel::Error:	return "\x1b[31m[Error]\x1b[37m";
		case LogLevel::Fatal:	return "\x1b[31m[Fatal]\x1b[37m";
		case LogLevel::Debug:	return "\x1b[32m[Debug]\x1b[37m";
		default:				return "\x1b[37m[Info]\x1b[37m";
		}
	}

	[[maybe_unused]]
	static std::string QueryLogTime()
	{
		return std::format("[{:%X}]", std::chrono::system_clock::now());
	}

	void Logger::WriteLog(LogLevel Level, std::string_view Message)
	{
		//std::println("{0} {1} {2}", QueryLogTime(), LogLevelToString(Level), Message);
		std::println("{0} {1}", LogLevelToString(Level), Message);
	}

	void Logger::WriteLog(LogLevel Level, std::wstring_view Message)
	{
		std::println("{0} {1}", LogLevelToString(Level), String::FromWide(Message));
	}
} // namespace Luden::Core
