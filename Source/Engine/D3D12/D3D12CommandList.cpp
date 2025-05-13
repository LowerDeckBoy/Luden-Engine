#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12CommandList.hpp"
#include <D3D12AgilitySDK/d3dx12/d3dx12.h>
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	D3D12CommandList::D3D12CommandList(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE CommandListType)
	{
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommandList1(pDevice->NodeMask, CommandListType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_GraphicsCommandList)));
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&m_CommandAllocator)));
		
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
		if (bIsOpen)
		{
			LOG_DEBUG("Command List is already open.");
			return S_OK;
		}

		HRESULT result = m_CommandAllocator->Reset();
		if (FAILED(result))
		{
			LOG_DEBUG("Failed to reset CommandAllocator!");
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

	void D3D12CommandList::Flush()
	{
		//VERIFY_D3D12_RESULT(m_CommandAllocator->Reset());
		VERIFY_D3D12_RESULT(m_GraphicsCommandList->Reset(m_CommandAllocator.Get(), nullptr));
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

	void D3D12CommandList::ResourcesTransition(const std::vector<std::pair<D3D12Resource*, D3D12_RESOURCE_STATES>>& pResources)
	{
		std::vector<CD3DX12_RESOURCE_BARRIER> barriers{};
		for (auto& resource : pResources)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource.first->GetHandleRaw(), resource.first->GetCurrentState(), resource.second));
			resource.first->SetResourceState(resource.second);
		}

		m_GraphicsCommandList->ResourceBarrier(static_cast<uint32>(barriers.size()), barriers.data());
	}

	void D3D12CommandList::CopyResource(D3D12Resource* pSource, D3D12Resource* pDestination)
	{
		m_GraphicsCommandList->CopyResource(pDestination->GetHandleRaw(), pSource->GetHandleRaw());
	}

	void D3D12CommandList::CopyBufferToBuffer(D3D12Buffer* pSource, D3D12Buffer* pDestination)
	{
		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData		= pDestination->GetBufferDesc().Data;
		subresource.RowPitch	= static_cast<LONG_PTR>(pDestination->GetBufferDesc().Size);
		subresource.SlicePitch	= subresource.RowPitch;

		::UpdateSubresources(GetHandleRaw(), pDestination->GetHandleRaw(), pSource->GetHandleRaw(), 0, 0, 1, &subresource);
	}

	void D3D12CommandList::CopyBufferToBuffer(D3D12Resource* pSource, D3D12Resource* pDestination, void* pData, uint64 Size)
	{
		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = pData;
		subresource.RowPitch = static_cast<LONG_PTR>(Size);
		subresource.SlicePitch = subresource.RowPitch;

		::UpdateSubresources(GetHandleRaw(), pDestination->GetHandleRaw(), pSource->GetHandleRaw(), 0, 0, 1, &subresource);
	}

	void D3D12CommandList::CopyTextureToTexture(D3D12Resource* pSource, D3D12Resource* pDestination, D3D12_SUBRESOURCE_DATA* Subresource)
	{
		::UpdateSubresources(GetHandleRaw(), pDestination->GetHandleRaw(), pSource->GetHandleRaw(), 0, 0, 1, Subresource);
	}

	void D3D12CommandList::SetRootSignature(D3D12RootSignature* pRootSignature)
	{
		if (pRootSignature->GetPipelineType() == PipelineType::Graphics)
		{
			m_GraphicsCommandList->SetGraphicsRootSignature(pRootSignature->GetHandleRaw());
		}
		else
		{
			m_GraphicsCommandList->SetComputeRootSignature(pRootSignature->GetHandleRaw());
		}
	}

	void D3D12CommandList::SetGraphicsRootSignature(D3D12RootSignature* pRootSignature)
	{
		m_GraphicsCommandList->SetGraphicsRootSignature(pRootSignature->GetHandleRaw());
	}

	void D3D12CommandList::SetComputeRootSignature(D3D12RootSignature* pRootSignature)
	{
		m_GraphicsCommandList->SetComputeRootSignature(pRootSignature->GetHandleRaw());
	}

	void D3D12CommandList::SetPipelineState(D3D12PipelineState* pPipelineState)
	{
		m_GraphicsCommandList->SetPipelineState(pPipelineState->GetHandleRaw());
	}

	void D3D12CommandList::ResolveSubresource(D3D12Resource* DestResource, uint32 DestSubresource, D3D12Resource* SourceResource, uint32 SourceSubresource, DXGI_FORMAT Format)
	{
		m_GraphicsCommandList->ResolveSubresource(DestResource->GetHandle(), DestSubresource, SourceResource->GetHandle(), SourceSubresource, Format);
	}

	void D3D12CommandList::ClearDepthStencilView(D3D12Descriptor& DepthStencilView)
	{
		m_GraphicsCommandList->ClearDepthStencilView(DepthStencilView.CpuHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);
	}

	void D3D12CommandList::SetRenderTarget(D3D12Descriptor& RenderTargetView)
	{
		m_GraphicsCommandList->OMSetRenderTargets(1, &RenderTargetView.CpuHandle, false, nullptr);
	}

	void D3D12CommandList::SetRenderTarget(D3D12Descriptor& RenderTargetView, D3D12Descriptor& DepthStencilView)
	{
		m_GraphicsCommandList->OMSetRenderTargets(1, &RenderTargetView.CpuHandle, false, &DepthStencilView.CpuHandle);
	}

	void D3D12CommandList::SetRenderTargets(const std::vector<D3D12Descriptor*>& RenderTargetViews, D3D12Descriptor& DepthStencilView)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
		for (auto view : RenderTargetViews)
		{
			//handles.push_back((D3D12_CPU_DESCRIPTOR_HANDLE&)view.CpuHandle);
			handles.push_back(view->CpuHandle);
		}

		m_GraphicsCommandList->OMSetRenderTargets(static_cast<uint32>(handles.size()), handles.data(), false, &DepthStencilView.CpuHandle);
	}

	void D3D12CommandList::ClearRenderTarget(D3D12Descriptor& RenderTargetView, std::array<float, 4> ClearColor)
	{
		m_GraphicsCommandList->ClearRenderTargetView(RenderTargetView.CpuHandle, ClearColor.data(), 0, nullptr);
	}

	void D3D12CommandList::ClearRenderTargets(const std::vector<D3D12Descriptor*>& RenderTargetViews, std::array<float, 4> ClearColor)
	{
		for (auto& view : RenderTargetViews)
		{
			m_GraphicsCommandList->ClearRenderTargetView(view->CpuHandle, ClearColor.data(), 0, nullptr);
		}
	}

	void D3D12CommandList::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
	{
		m_GraphicsCommandList->IASetPrimitiveTopology(PrimitiveTopology);
	}

	void D3D12CommandList::PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset)
	{
		if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			m_GraphicsCommandList->SetGraphicsRoot32BitConstants(Slot, Count, pData, Offset);
		}
		else if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			m_GraphicsCommandList->SetComputeRoot32BitConstants(Slot, Count, pData, Offset);
		}
	}

	void D3D12CommandList::PushRootSRV(uint32 Slot, uint64 Address)
	{
		m_GraphicsCommandList->SetGraphicsRootShaderResourceView(Slot, (D3D12_GPU_VIRTUAL_ADDRESS)Address);
	}

	void D3D12CommandList::DispatchMesh(uint32 DispatchThreadX, uint32 DispatchThreadY, uint32 DispatchThreadZ)
	{
		m_GraphicsCommandList->DispatchMesh(DispatchThreadX, DispatchThreadY, DispatchThreadZ);
	}

	void D3D12CommandList::Draw(uint32 VertexCount)
	{
		m_GraphicsCommandList->DrawInstanced(VertexCount, 1, 0, 0);
	}

	void D3D12CommandList::DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex)
	{
		m_GraphicsCommandList->DrawIndexedInstanced(IndexCount, 1, BaseIndex, BaseVertex, 0);
	}

	void D3D12CommandList::SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW IndexBufferView)
	{
		m_GraphicsCommandList->IASetIndexBuffer(&IndexBufferView);
	}

	void D3D12CommandList::SetIndexBuffer(D3D12Buffer* pIndexBuffer)
	{
		auto view = D3D12_INDEX_BUFFER_VIEW(
			pIndexBuffer->GetHandleRaw()->GetGPUVirtualAddress(),
			static_cast<uint32>(pIndexBuffer->GetBufferDesc().Size),
			DXGI_FORMAT_R32_UINT);
		m_GraphicsCommandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandList::SetConstantBuffer(uint32 RegisterSlot, D3D12ConstantBuffer* pConstantBuffer)
	{
		const auto address = pConstantBuffer->GetBuffer()->GetGPUVirtualAddress();

		if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			m_GraphicsCommandList->SetGraphicsRootConstantBufferView(RegisterSlot, address);
		}
		else if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			m_GraphicsCommandList->SetComputeRootConstantBufferView(RegisterSlot, address);
		}
	}

} // namespace Luden
