#include "Logger.hpp"
#include "Assert.hpp"
#include "Defines.hpp"

namespace Luden::Core
{
	static std::string ExtractFilename(const char* File)
	{
		return strrchr(File, '\\') ? strrchr(File, '\\') + 1 : File;
	}

	std::string Assert::GetWhereCalled(const std::source_location & Location)
	{
		return std::format("\tFile: {0} ({1}:{2})\n\tFunction: {3}",
			ExtractFilename(Location.file_name()),
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
		__debugbreak();
	#else
		// Temporal
		throw std::expection("");
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
		__debugbreak();
	#else
		// Temporal
		throw std::expection("");
	#endif

	}
} // namespace Luden::Core
