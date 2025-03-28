#include "Config.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Memory.hpp"
#include <Core/Logger.hpp>
#include <iostream>

namespace Luden
{
	D3D12Adapter::D3D12Adapter()
	{
		uint32 dxgiFactoryFlag = 0;
		if (Config::Get().bEnableDebugLayer)
		{
			VERIFY_D3D12_RESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugDevice)));
			m_DebugDevice->EnableDebugLayer();

			dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
		}
		else
		{
			LOG_DEBUG("D3D12 Debug Layer is disabled.");
		}
		
		VERIFY_D3D12_RESULT(CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&Factory)));

		for (uint32 i = 0;
			Factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter)) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), MinRequiredFeatureLevel, __uuidof(ID3D12Device14), nullptr)))
			{
				Adapter->GetDesc3(&Desc);

				break;
			}
		}

		LOG_INFO(L"GPU: {0} {1} GB.", Desc.Description, (Desc.DedicatedVideoMemory / 1024L / 1024L / 1000L));

	}

	D3D12Adapter::~D3D12Adapter()
	{
		SAFE_RELEASE(Adapter);
		SAFE_RELEASE(Factory);
	}

	uint64 D3D12Adapter::QueryAdapterMemory()
	{
		Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &MemoryInfo);
		
		return BYTES_TO_MEGABYTES(MemoryInfo.CurrentUsage);
	}
	f64 D3D12Adapter::GetCurrentMemoryUsage()
	{
		Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &MemoryInfo);

		return MemoryInfo.CurrentUsage / 1024.0 / 1024.0 / 1024.0;
	}
	f64 D3D12Adapter::GetDeviceTotalMemory() const
	{
		return MemoryInfo.Budget / 1024.0 / 1024.0 / 1024.0;
	}
} // namespace Luden
