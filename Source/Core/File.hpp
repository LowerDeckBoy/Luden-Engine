#pragma once

#include "Export.hpp"
#include "String.hpp"
#include <filesystem>

using Filepath = std::filesystem::path;

namespace Luden::File
{
	inline CORE_API bool Exists(Filepath Path)
	{
		return std::filesystem::exists(Path);
	}

	inline CORE_API std::string GetFilename(Filepath Path)
	{
		return Path.filename().string();
	}

	inline CORE_API std::string GetRelativePath(Filepath Path)
	{
		return Path.relative_path().string();
	}

	inline CORE_API std::string GetParentPath(Filepath Path)
	{
		return std::filesystem::path(Path).parent_path().string();
	}
} // namespace Luden::File
