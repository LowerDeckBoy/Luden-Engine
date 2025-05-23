#pragma once

#include "Defines.hpp"
#include "Export.hpp"
#include <string>

namespace Luden::String
{
	CORE_API INLINE std::wstring ToWide(std::string Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	CORE_API INLINE std::wstring ToWide(std::string_view Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	CORE_API INLINE std::string FromWide(std::wstring Text)
	{
		std::string narrowed{};
		narrowed.resize(Text.length());

		wcstombs_s(nullptr, &narrowed[0], narrowed.size() + 1, Text.data(), Text.size());

		return narrowed;
	}

	CORE_API INLINE std::string FromWide(std::wstring_view Text)
	{
		std::string narrowed{};
		narrowed.resize(Text.length());

		wcstombs_s(nullptr, &narrowed[0], narrowed.size() + 1, Text.data(), Text.size());

		return narrowed;
	}

	// Check whether two given const char* are the same.
	CORE_API INLINE bool Compare(const char* String1, const char* String2)
	{
		return std::strcmp(String1, String2);
	}

} // namespace Luden::String
