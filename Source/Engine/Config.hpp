#pragma once

#include <Core/Types.hpp>

namespace Luden
{
	struct Config
	{
		static Config& Get()
		{
			static Config config;

			return config;
		}

		bool bEnableDebugLayer = true;

		// Either 2 or 3.
		uint32 NumBackBuffers = 2;

		// 0 - VSync off
		// 1 - VSync on
		// 2 - VSync half
		uint32 SyncInterval = 0;

		bool Raytracing = false;
		bool MeshShading = false;

	};
} // namespace Luden
