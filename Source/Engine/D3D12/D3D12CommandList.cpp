#include "D3D12Device.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	D3D12CommandList::D3D12CommandList(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE CommandListType)
	{
		VERIFY_D3D12_RESULT(pDevice->Device->CreateCommandList1(pDevice->NodeMask, CommandListType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_GraphicsCommandList)));
		VERIFY_D3D12_RESULT(pDevice->Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&m_CommandAllocator)));

		m_CommandListType = CommandListType;

		// Set debug names for given type of commands.
		switch (CommandListType)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			NAME_D3D12_OBJECT(m_GraphicsCommandList.Get(),	"D3D12 Direct Command List");
			NAME_D3D12_OBJECT(m_CommandAllocator.Get(),		"D3D12 Direct Command Allocator");
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			NAME_D3D12_OBJECT(m_GraphicsCommandList.Get(),	"D3D12 Compute Command List");
			NAME_D3D12_OBJECT(m_CommandAllocator.Get(),		"D3D12 Compute Command Allocator");
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			NAME_D3D12_OBJECT(m_GraphicsCommandList.Get(),	"D3D12 Copy Command List");
			NAME_D3D12_OBJECT(m_CommandAllocator.Get(),		"D3D12 Copy Command Allocator");
			break;
		}
	}

	D3D12CommandList::~D3D12CommandList()
	{
		SAFE_RELEASE(m_CommandAllocator);
		SAFE_RELEASE(m_GraphicsCommandList);
	}

	HRESULT D3D12CommandList::Open()
	{
		HRESULT result = m_CommandAllocator->Reset();
		if (FAILED(result))
		{
			return result;
		}

		result = m_GraphicsCommandList->Reset(m_CommandAllocator.Get(), nullptr);
		if (SUCCEEDED(result))
		{
			bIsOpen = true;
		}

		return result;
	}

	HRESULT D3D12CommandList::Close()
	{
		if (!bIsOpen)
		{
			LOG_DEBUG("Command List is already closed.");

			return S_OK;
		}

		HRESULT hResult = m_GraphicsCommandList->Close();

		if (SUCCEEDED(hResult))
		{
			bIsOpen = false;
		}

		return hResult;
	}

	void D3D12CommandList::SetDescriptorHeap(D3D12DescriptorHeap* pDescriptorHeap)
	{
		ID3D12DescriptorHeap* heaps[1] = { pDescriptorHeap->GetHandleRaw() };
		m_GraphicsCommandList->SetDescriptorHeaps(1, &heaps[0]);

	}

	void D3D12CommandList::SetViewport(D3D12Viewport* pViewport)
	{
		m_GraphicsCommandList->RSSetViewports(1, &pViewport->Viewport);
		m_GraphicsCommandList->RSSetScissorRects(1, &pViewport->Scissor);
	}

	void D3D12CommandList::ResourceTransition(D3D12Resource* pResource, D3D12_RESOURCE_STATES After)
	{
		if (pResource->GetCurrentState() == After)
		{
			LOG_WARNING("Skipped Resource barrier.\n");
			return;
		}

		const auto& barrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource->GetHandleRaw(), pResource->GetCurrentState(), After);
		m_GraphicsCommandList->ResourceBarrier(1, &barrier);
		pResource->SetResourceState(After);
	}

	void D3D12CommandList::CopyResource(D3D12Resource* pSource, D3D12Resource* pDestination)
	{
		m_GraphicsCommandList->CopyResource(pDestination->GetHandleRaw(), pSource->GetHandleRaw());
	}

} // namespace Luden
