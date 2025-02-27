#pragma once

#include "String.hpp"
#include <format>

namespace Luden::Core
{
	#define LOG_INFO(Message, ...)		Luden::Core::Logger::Log(Luden::Core::LogLevel::Info, Message, __VA_ARGS__)
	#define LOG_WARNING(Message, ...)	Luden::Core::Logger::Log(Luden::Core::LogLevel::Warning, Message, __VA_ARGS__)
	#define LOG_ERROR(Message, ...)		Luden::Core::Logger::Log(Luden::Core::LogLevel::Error, Message, __VA_ARGS__)
	#define LOG_FATAL(Message, ...)		Luden::Core::Logger::Log(Luden::Core::LogLevel::Fatal, Message, __VA_ARGS__)

#if defined CORE_DEBUG
	#define LOG_DEBUG(Message, ...)		Luden::Core::Logger::Log(Luden::Core::LogLevel::Debug, Message, __VA_ARGS__)
#else
	#define LOG_DEBUG(Message, ...)
#endif

	enum class LogLevel
	{
		Info,
		Warning,
		Error,
		Fatal,
		Debug
	};
	
	class CORE_API Logger
	{
	public:
		Logger() = default;
		~Logger() = default;

		template<typename... LogArgs>
		static void Log(LogLevel Level, std::string_view Message, LogArgs&&... Args)
		{
			const auto fmt = std::vformat(Message, std::make_format_args(std::forward<LogArgs>(Args)...));
			WriteLog(Level, fmt);
		}

		template<typename... LogArgs>
		static void Log(LogLevel Level, std::wstring_view Message, LogArgs&&... Args)
		{
			const auto fmt = std::vformat(Message, std::make_wformat_args(std::forward<LogArgs>(Args)...));
			WriteLog(Level, fmt);
		}

	private:
		static void WriteLog(LogLevel Level, std::string_view Message);
		static void WriteLog(LogLevel Level, std::wstring_view Message);

	};
} // namespace Luden::Core
