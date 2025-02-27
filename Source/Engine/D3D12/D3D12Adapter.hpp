#pragma once

#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <D3D12AgilitySDK/d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include <D3D12MemoryAllocator/D3D12MemAlloc.h>

namespace Luden
{
	class D3D12Adapter
	{
	public:
		D3D12Adapter();
		~D3D12Adapter();
		
		Ref<IDXGIFactory7> Factory;
		Ref<IDXGIAdapter4> Adapter;

		Ref<ID3D12Device14> LogicalDevice;

		void CreateFactory();
		void CreateAdapter();
		void CreateDevice();

		DXGI_ADAPTER_DESC3 Desc{};
		
		DXGI_QUERY_VIDEO_MEMORY_INFO MemoryInfo{};
		uint64 AvailableMemory = 0;

		// Minimal level of shader model that is required by D3D12 Render Hardware Interface
		// in order to support for bindless approach of HLSL 
		// ResourceDescriptorHeap[] and SamplerDescriptorHeap[] 
		D3D_SHADER_MODEL const MinRequiredShaderModel = D3D_SHADER_MODEL_6_6;

		D3D_SHADER_MODEL MaxSupportedShaderModel = D3D_SHADER_MODEL_NONE;

		D3D_FEATURE_LEVEL MinRequiredFeatureLevel = D3D_FEATURE_LEVEL_12_2;

		uint64 QueryAdapterMemory() const;

	private:
		Ref<ID3D12Debug6> m_DebugDevice;

	};

} // namespace Luden
