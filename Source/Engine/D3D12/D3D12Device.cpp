#include <Core/Logger.hpp>
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
	D3D12Device::D3D12Device(D3D12Adapter* pParentAdapter)
		: ParentAdapter(pParentAdapter)
	{
		VERIFY_D3D12_RESULT(D3D12CreateDevice(ParentAdapter->Adapter, ParentAdapter->MinRequiredFeatureLevel, IID_PPV_ARGS(&Device)));
		NAME_D3D12_OBJECT(Device.Get(), "D3D12 Logical Device");

		if (bEnableDebugLayer)
		{
			VERIFY_D3D12_RESULT(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_DXGIDebug)));
			VERIFY_D3D12_RESULT(Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue)));
			m_InfoQueue->RegisterMessageCallback(&DebugCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_DebugCallbackCookie);
		}
		
	}

	D3D12Device::~D3D12Device()
	{
		SAFE_RELEASE(Device);
		SAFE_RELEASE(m_InfoQueue);

		if (bEnableDebugLayer)
		{
			m_DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL)));
			SAFE_RELEASE(m_DXGIDebug);
		}
	}

	void D3D12Device::DebugCallback(
		D3D12_MESSAGE_CATEGORY	/* Category */,
		D3D12_MESSAGE_SEVERITY	Severity,
		D3D12_MESSAGE_ID		/* ID */,
		LPCSTR					pDescription,
		void*                   /* pContext */)
	{
		switch (Severity)
		{
		case D3D12_MESSAGE_SEVERITY_INFO:
			FALLTHROUGH;
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
			LOG_INFO("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_WARNING:
			LOG_WARNING("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_ERROR:
			LOG_ERROR("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
			LOG_FATAL("[D3D12] {}", pDescription);
			break;
		}
	}
} // namespace Luden
