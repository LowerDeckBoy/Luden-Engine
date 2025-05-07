#include "Logger.hpp"
#include "Assert.hpp"
#include "Defines.hpp"
#include "File.hpp"

namespace Luden::Core
{
	std::string Assert::GetWhereCalled(const std::source_location& Location)
	{
		return std::format("File: {0} ({1}:{2})\nFunction: {3}",
			File::ExtractFilename(Location.file_name()),
			Location.line(),
			Location.column(),
			Location.function_name()
		);
	}

	void Assert::MakeAssertion(bool Expression, const char* StringExpression, std::source_location SourceLocation)
	{
		if (Expression)
		{
			return;
		}

		auto message = std::format("Assertion failed: {0}\n{1}",
			StringExpression,
			GetWhereCalled(SourceLocation)
		);

		LOG_FATAL("{0}", message);

	#if CORE_DEBUG
		DEBUGBREAK();
	#else
		// Temporal
		throw std::runtime_error("");
	#endif

	}

	void Assert::MakeAssertionWithMessage(bool Expression, const char* StringExpression, std::source_location SourceLocation, std::string_view Message)
	{
		if (Expression)
		{
			return;
		}

		auto message = std::format("Assertion failed: {0}\n{1}\n{2}",
			StringExpression,
			Message,
			GetWhereCalled(SourceLocation)
		);

		LOG_FATAL("{0}\n\n{1}", Message, message);

	#if CORE_DEBUG
		DEBUGBREAK();
	#else
		// Temporal
		throw std::runtime_error("");
	#endif

	}
} // namespace Luden::Core
