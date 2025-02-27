#pragma once

#include <Core/File.hpp>
#include <Core/String.hpp>
#include <Core/Types.hpp>

namespace Luden
{
	using AssetId = uint32;

	constexpr uint32 InvalidAssetId = 0xFFFFFFFF;

	class Asset
	{
	public:

		AssetId ID;

		std::string Name;


	private:
		Filepath m_Filepath;

	};
} // namespace Luden
