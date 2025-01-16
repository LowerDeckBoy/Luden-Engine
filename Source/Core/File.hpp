#pragma once

#include "String.hpp"
#include <filesystem>

using Filepath = std::filesystem::path;

namespace Luden::File
{
	inline bool Exists(Filepath Path)
	{
		return std::filesystem::exists(Path);
	}

	inline std::string GetFilename(Filepath Path)
	{
		return Path.filename().string();
	}

	inline std::string GetRelativePath(Filepath Path)
	{
		return Path.relative_path().string();
	}

	inline std::string GetParentPath(Filepath Path)
	{
		return std::filesystem::path(Path).parent_path().string();
	}
} // namespace Luden::File
