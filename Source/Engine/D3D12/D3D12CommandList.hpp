#pragma once

#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <array>

namespace Luden
{
	class D3D12Device;
	class D3D12Descriptor;
	class D3D12DescriptorHeap;
	class D3D12Viewport;
	class D3D12Resource;
	class D3D12RootSignature;
	class D3D12PipelineState;
	class D3D12ConstantBuffer;

	constexpr std::array<float, 4> DefaultClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	class D3D12CommandList
	{
	public:
		D3D12CommandList(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE CommandListType);
		~D3D12CommandList();

		Ref<ID3D12GraphicsCommandList10>& GetHandle()
		{
			return m_GraphicsCommandList;
		}

		ID3D12GraphicsCommandList10* GetHandleRaw()
		{
			return m_GraphicsCommandList.Get();
		}

		Ref<ID3D12CommandAllocator>& GetAllocator()
		{
			return m_CommandAllocator;
		}

		HRESULT Open();
		HRESULT Close();

		// Reset CommandList and it's Allocator without signaling
		// CommandList as Open.
		void Flush();

		bool IsOpen() const
		{
			return bIsOpen;
		}
		
		// TODO: Add Sampler Heap binding.
		// nullptr as default.
		void SetDescriptorHeap(D3D12DescriptorHeap* pDescriptorHeap);

		void SetViewport(D3D12Viewport* pViewport);

		// Before state is taken from pResource itself.
		void ResourceTransition(D3D12Resource* pResource, D3D12_RESOURCE_STATES After);

		// Either Buffer to Buffer or Texture to Texture.
		void CopyResource(D3D12Resource* pSource, D3D12Resource* pDestination);

		void CopyBufferToBuffer(D3D12Buffer* pSource, D3D12Buffer* pDestination);
		void CopyBufferToBuffer(D3D12Resource* pSource, D3D12Resource* pDestination, void* pData, uint64 Size);

		void SetRootSignature(D3D12RootSignature* pRootSignature);

		void SetPipelineState(D3D12PipelineState* pPipelineState);

		void ClearDepthStencilView(D3D12Descriptor& DepthStencilView);

		void SetRenderTarget(D3D12Descriptor& RenderTargetView);
		void SetRenderTarget(D3D12Descriptor& RenderTargetView, D3D12Descriptor& DepthStencilView);

		void ClearRenderTarget(D3D12Descriptor& RenderTargetView, std::array<float, 4> ClearColor = DefaultClearColor);

		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);

		void PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset = 0);
		void PushRootSRV(uint32 Slot, uint64 Address);

		void DispatchMesh(uint32 DispatchThreadX, uint32 DispatchThreadY, uint32 DispatchThreadZ);

		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex);

		//void SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW* pIndexBufferView);
		void SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW IndexBufferView);
		void SetIndexBuffer(D3D12Buffer* pIndexBuffer);
		void SetConstantBuffer(uint32 RegisterSlot, D3D12ConstantBuffer* pConstantBuffer);
		
	private:
		Ref<ID3D12GraphicsCommandList10>	m_GraphicsCommandList;
		Ref<ID3D12CommandAllocator>			m_CommandAllocator;
		D3D12_COMMAND_LIST_TYPE				m_CommandListType;
		
		bool bIsOpen = false;

	}; // class D3D12CommandList

	class D3D12CommandSignature
	{
	public:
		D3D12CommandSignature(D3D12Device* pDevice);
		~D3D12CommandSignature();

		Ref<ID3D12CommandSignature>&	GetHandle()		{ return m_CommandSignature;		}
		ID3D12CommandSignature*			GetHandleRaw()	{ return m_CommandSignature.Get();	}

	private:
		Ref<ID3D12CommandSignature> m_CommandSignature;

	};
} // namespace Luden
