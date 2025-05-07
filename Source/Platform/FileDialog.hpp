#pragma once

#include "Export.hpp"

namespace Luden::Platform
{
	// Rename to *Extensions*?
	// Should these be maps?
	namespace Filters
	{
		// Used for top-most filter.
		// Groups all extensions for given output into single filter.
		// Excludes *Any* filter.
		constexpr LPCWSTR Primary	= L"";

		constexpr LPCWSTR JSON	= L"JSON";

		constexpr LPCWSTR GLTF	= L"glTF 2.0";
		constexpr LPCWSTR FBX	= L"FBX";
		constexpr LPCWSTR OBJ	= L"Wavefront";

		constexpr LPCWSTR JPG	= L"JPG";
		constexpr LPCWSTR PNG	= L"PNG";
		constexpr LPCWSTR DDS	= L"DDS";

		// Accepts all extensions.
		// Used as last filter.
		constexpr LPCWSTR Any	= L"Any";
		
	}

	enum class EExtensionFilter
	{
		Scene,
		Model,
		Image,
		Any
	};

	struct FOpenDialogOptions
	{
		EExtensionFilter FilterExtensions = EExtensionFilter::Any;

		// Default location for this dialog.
		std::string	OpenLocation;
		std::string	Title = "Select file";

	};
	
	// TODO:
	struct FSaveDialogOptions
	{

	};

	class PLATFORM_API FileDialog
	{
	public:

		static std::string Open(FOpenDialogOptions Options);

		// TODO:
		//static std::string Save();

	};
} // namespace Luden::Platform
