#pragma once

#include "Export.hpp"
#include "String.hpp"
#include <filesystem>

using Filepath = std::filesystem::path;

namespace Luden::File
{
	CORE_API inline bool Exists(Filepath Path)
	{
		return std::filesystem::exists(Path);
	}

	CORE_API inline std::string GetExtension(Filepath Path)
	{
		return Path.extension().string();
	}

	CORE_API inline std::string GetRelativePath(Filepath Path)
	{
		return Path.relative_path().string();
	}

	CORE_API inline std::string GetParentPath(Filepath Path)
	{
		return Path.parent_path().string();
	}

	// Return filename without it's extension.
	CORE_API inline std::string GetFilename(Filepath Path)
	{
		return Path.stem().string();
	}

	// Return filename without it's extension.
	CORE_API inline std::string ExtractFilename(const char* File)
	{
		return std::string(strrchr(File, '\\') ? strrchr(File, '\\') + 1 : File);
	}
} // namespace Luden::File
