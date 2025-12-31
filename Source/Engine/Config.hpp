#pragma once

/*=================================================================================
	Engine/Config.hpp

	Static struct of helper options used across Engine and Editor.
=================================================================================*/

#include <Core/Types.hpp>

namespace Luden
{
	// TODO:
	// Config requires either repacking to keep 4 byte strides
	// or changing types from bools to uint32.
	struct Config
	{
		static Config& Get()
		{
			static Config config;

			return config;
		}

		// Note: don't change during runtime.
		// Either 2 or 3.
		uint32 NumBackBuffers = 2;

		// DXGI Sync Intervals:
		// 0 - VSync off
		// 1 - VSync on
		// 2 - VSync half
		// 3 - VSync third
		// 4 - VSync quarter
		int32 SyncInterval = 2;

		bool bEnableDebugLayer = true;

		// If SyncInterval == 0.
		// Frame limit can be set to value in range [24; 240].
		bool bAllowFixedFrameRate = false;

		// TODO:
		// Clamp frame rate to desired [min;max] range.
		bool bAllowSmoothFrameRate = false;

		// If false - switch to vertex shading.
		bool bMeshShading = true;
		
		bool bDrawMeshlets = false;

		bool bMeshletCulling = true;

		// Debug only.
		// Whether to enable alpha masking cutoff in pixel shaders.
		bool bAlphaMask = true;

		// Temporarly
		//bool bLightPassCompute = true;
		bool padding;
		bool bDrawIndirect = false;

		// True, to dispatch Raytracing.
		// Else, use raster mode.
		bool bRaytracing = false;

		bool bHideEditor = false;
		
		bool bEnableSSAO = false;

		bool bEnableSky = true;
		bool bEnableAtmosphere = false;

		bool bEnableSSR = false;

		/* =============== Post - Processing =============== */
		// Whether to enable PostProcessing or not.
		bool bEnablePostProcess	= true;
		bool bEnableBloom		= true;
		bool bEnableTonemapping = true;
		bool bEnableFXAA		= true;


	};
} // namespace Luden
