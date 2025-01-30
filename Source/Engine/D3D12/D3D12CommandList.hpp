#pragma once

#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>

namespace Luden
{
	class D3D12Device;
	class D3D12DescriptorHeap;
	class D3D12Viewport;
	class D3D12Resource;

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

		bool IsOpen() const
		{
			return bIsOpen;
		}
		
		// TODO: Add Sampler Heap binding.
		// nullptr as default.
		void SetDescriptorHeap(D3D12DescriptorHeap* pDescriptorHeap);

		void SetViewport(D3D12Viewport* pViewport);
		//void SetViewport(D3D12_VIEWPORT& Viewport);

		void ResourceTransition(D3D12Resource* pResource, D3D12_RESOURCE_STATES After);

		void CopyResource(D3D12Resource* pSource, D3D12Resource* pDestination);

	private:
		Ref<ID3D12GraphicsCommandList10>	m_GraphicsCommandList;
		Ref<ID3D12CommandAllocator>			m_CommandAllocator;
		
		D3D12_COMMAND_LIST_TYPE m_CommandListType;

		bool bIsOpen = false;

	};
} // namespace Luden
