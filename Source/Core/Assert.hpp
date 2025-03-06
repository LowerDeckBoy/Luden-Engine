#pragma once

#include "Export.hpp"
#include <cassert>
#include <source_location>

#define ASSERT(Expression) \
		Luden::Core::Assert::MakeAssertion(Expression, #Expression, std::source_location::current())

#define ASSERT_LOG(Expression, ...) \
		Luden::Core::Assert::MakeAssertionWithMessage(Expression, #Expression, std::source_location::current(), __VA_ARGS__)

namespace Luden::Core
{
	class CORE_API Assert
	{
	public:
		Assert() = default;
		~Assert() = default;

		static void MakeAssertion(bool Expression, const char* StringExpression, std::source_location SourceLocation);

		static void MakeAssertionWithMessage(bool Expression, const char* StringExpression, std::source_location SourceLocation, std::string_view Message);
		
		static std::string GetWhereCalled(const std::source_location& Location);

	};
} // namespace Luden::Core
