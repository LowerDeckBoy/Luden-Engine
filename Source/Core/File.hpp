#pragma once

#include "Export.hpp"
#include <filesystem>
#include <string>

using Filepath = std::filesystem::path;

namespace Luden::File
{
	inline bool CORE_API Exists(Filepath Path)
	{
		return std::filesystem::exists(Path);
	}

	inline std::string CORE_API GetFilename(Filepath Path)
	{
		return Path.filename().string();
	}

	inline std::string CORE_API GetExtension(Filepath Path)
	{
		return Path.extension().string();
	}

	inline std::string CORE_API GetRelativePath(Filepath Path)
	{
		return Path.relative_path().string();
	}

	inline std::string CORE_API GetParentPath(Filepath Path)
	{
		return Path.parent_path().string();
	}
} // namespace Luden::File
