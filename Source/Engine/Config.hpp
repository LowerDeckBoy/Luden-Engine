#pragma once

/*=================================================================================
	Engine/Config.hpp

	Static struct of helper options used across Engine and Editor.
=================================================================================*/

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

		// Note: don't change during runtime.
		// Either 2 or 3.
		uint32 NumBackBuffers = 2;

		// DXGI Sync Intervals:
		// 0 - VSync off
		// 1 - VSync on
		// 2 - VSync half
		// 3 - VSync third
		// 4 - VSync quarter
		int32 SyncInterval = 1;

		// If SyncInterval == 0.
		// Frame limit can be set to value in range [24; 240].
		bool bAllowFixedFrameRate = false;

		// TODO:
		// Clamp frame rate to desired [min;max] range.
		bool bAllowSmoothFrameRate = false;

		// If false - switch to vertex shading.
		bool bMeshShading = true;
		
		bool bDrawMeshlets = true;

		// True, to dispatch Raytracing.
		// Else, use raster mode.
		bool bRaytracing = false;

		// Debug only.
		// Whether to enable alpha masking cutoff in pixel shaders.
		bool bAlphaMask = true;

		bool bDrawSky = false;

		// Debug grid for Editor Mode.
		// Placeholder for now.
		bool bDrawGrid = false;

	};
} // namespace Luden
