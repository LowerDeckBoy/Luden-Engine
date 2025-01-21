#include "D3D12Adapter.hpp"
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>
#include <iostream>

namespace Luden
{
	D3D12Adapter::D3D12Adapter()
	{

		VERIFY_D3D12_RESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugDevice)));
		m_DebugDevice->EnableDebugLayer();

		VERIFY_D3D12_RESULT(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&Factory)));

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

		const SIZE_T vram = static_cast<SIZE_T>(Desc.DedicatedVideoMemory / 1024L / 1024L / 1000L);
		LOG_INFO(L"GPU: {} {}GB", Desc.Description, vram);

	}

	D3D12Adapter::~D3D12Adapter()
	{
		SAFE_RELEASE(Adapter);
		SAFE_RELEASE(Factory);
	}
} // namespace Luden
