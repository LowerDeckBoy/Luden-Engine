#pragma once

#include "D3D12Adapter.hpp"

namespace Luden
{
	class D3D12Device
	{
	public:
		explicit D3D12Device(D3D12Adapter* pParentAdapter);
		~D3D12Device();

		D3D12Adapter* ParentAdapter;

		Ref<ID3D12Device14> Device;

		Ref<D3D12MA::Allocator> D3D12MemoryAllocator;

		uint32 NodeMask = 0;

		// Temp
		bool bEnableDebugLayer = true;
	private:
		Ref<IDXGIDebug1>		m_DXGIDebug;

		Ref<ID3D12InfoQueue1>	m_InfoQueue;
		DWORD					m_DebugCallbackCookie = 0;

		static void DebugCallback(
			D3D12_MESSAGE_CATEGORY	Category,
			D3D12_MESSAGE_SEVERITY	Severity,
			D3D12_MESSAGE_ID		ID,
			LPCSTR					pDescription,
			void* pContext);
	};
} // namespace Luden
