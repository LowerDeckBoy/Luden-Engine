#pragma once

#include "Config.hpp"

#include "D3D12Adapter.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Raytracing.hpp"
#include "D3D12Texture.hpp"

namespace Luden
{
	class D3D12Device
	{
	public:
		explicit D3D12Device(D3D12Adapter* pParentAdapter);
		~D3D12Device();

		D3D12Adapter*			ParentAdapter;
		Ref<ID3D12Device14>		LogicalDevice;
		Ref<D3D12MA::Allocator> D3D12MemoryAllocator;

		uint32 NodeMask = 0;

		void QueryDeviceFeatures();
		
		uint32 CreateBuffer(BufferDesc Desc);
		uint32 CreateConstantBuffer(void* pData, usize Size);

		std::vector<D3D12Buffer*> Buffers;
		std::vector<D3D12ConstantBuffer*> ConstantBuffers;

		INLINE D3D12Buffer* GetBuffer(uint32 Index)
		{
			return Buffers.at(Index);
		}

		// Create SRV for Texture usage.
		void CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips = 1, uint32 Count = 1);
		// Create SRV for Buffer usage.
		void CreateShaderResourceView(D3D12Buffer* pBuffer);
		void CreateUnorderedAccessView(D3D12Texture* pTexture);
		void CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count = 1);
		void CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		D3D12DescriptorHeap* ShaderResourceHeap;
		D3D12DescriptorHeap* RenderTargetHeap;
		D3D12DescriptorHeap* DepthStencilHeap;

	private:
		Ref<IDXGIDebug1>		m_DXGIDebug;
		Ref<ID3D12DebugDevice2> m_DebugDevice2;
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
