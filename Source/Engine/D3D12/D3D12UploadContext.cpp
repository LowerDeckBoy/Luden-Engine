#include "D3D12RHI.hpp"
#include "D3D12UploadContext.hpp"
#include "D3D12Memory.hpp"
#include "D3D12Utility.hpp"
#include <D3D12AgilitySDK/d3dx12/d3dx12.h>

#include "Core/Logger.hpp"

namespace Luden
{
	D3D12RHI*			D3D12UploadContext::m_D3D12RHI = nullptr;
	D3D12CommandQueue*	D3D12UploadContext::UploadQueue = nullptr;
	D3D12CommandList*	D3D12UploadContext::CommandList = nullptr;

	std::vector<UploadResourceRequest> D3D12UploadContext::PendingRequests;

	void D3D12UploadContext::Create(D3D12RHI* pD3D12RHI)
	{
		m_D3D12RHI = pD3D12RHI;

		UploadQueue = new D3D12CommandQueue(pD3D12RHI->Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		CommandList = new D3D12CommandList(pD3D12RHI->Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	void D3D12UploadContext::Release()
	{
		if (CommandList)
		{
			delete CommandList;
		}

		if (UploadQueue)
		{
			delete UploadQueue;
		}
	}

	void D3D12UploadContext::Upload()
	{
		if (PendingRequests.empty())
		{
			return;
		}

		CommandList->Open();

		for (auto& request : PendingRequests)
		{
			if (request.UploadType == Buffer)
			{
				CommandList->ResourceTransition(request.Resource, D3D12_RESOURCE_STATE_COPY_DEST);
				CommandList->ResourceTransition(request.UploadResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
				CommandList->CopyResource(request.UploadResource, request.Resource);
				CommandList->ResourceTransition(request.Resource, D3D12_RESOURCE_STATE_GENERIC_READ);
				CommandList->ResourceTransition(request.UploadResource, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
			else if (request.UploadType == Texture)
			{
				CommandList->ResourceTransition(request.Resource, D3D12_RESOURCE_STATE_COPY_DEST);
				CommandList->ResourceTransition(request.UploadResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
				CommandList->CopyTextureToTexture(request.UploadResource, request.Resource, &request.Resource->Subresource);
				CommandList->ResourceTransition(request.Resource, D3D12_RESOURCE_STATE_GENERIC_READ);
				CommandList->ResourceTransition(request.UploadResource, D3D12_RESOURCE_STATE_GENERIC_READ);
			}

		}

		UploadQueue->Execute({ CommandList });

		UploadQueue->Fence.CurrentValue++;
		UploadQueue->Signal();

		if (SUCCEEDED(UploadQueue->Fence.Fence->SetEventOnCompletion(UploadQueue->Fence.CurrentValue, UploadQueue->Fence.FenceEvent)))
		{
			if (::WaitForSingleObject(UploadQueue->Fence.FenceEvent, 3'000) == WAIT_TIMEOUT)
			{
				LOG_FATAL("GPU Timeout (D3D12UploadContext::Upload())");
			}
		}

		for (auto& request : PendingRequests)
		{
			request.UploadResource->Release();
		}

		PendingRequests.clear();

	}

	void D3D12UploadContext::UploadTexture(D3D12Texture* pTexture, D3D12Resource* pUploadBuffer)
	{
		CommandList->Open();

		CommandList->ResourceTransition(pTexture, D3D12_RESOURCE_STATE_COPY_DEST);
		CommandList->ResourceTransition(pUploadBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
		CommandList->CopyTextureToTexture(pUploadBuffer, pTexture, &pTexture->Subresource);
		CommandList->ResourceTransition(pTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
		CommandList->ResourceTransition(pUploadBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

		UploadQueue->Execute({ CommandList });

		UploadQueue->Fence.CurrentValue++;
		UploadQueue->Signal();

		if (SUCCEEDED(UploadQueue->Fence.Fence->SetEventOnCompletion(UploadQueue->Fence.CurrentValue, UploadQueue->Fence.FenceEvent)))
		{
			if (::WaitForSingleObject(UploadQueue->Fence.FenceEvent, 3'000) == WAIT_TIMEOUT)
			{
				LOG_FATAL("GPU Timeout (D3D12UploadContext::Upload())");
			}
		}

		pUploadBuffer->Release();
	}

	void D3D12UploadContext::UploadBuffer(D3D12Buffer* pBuffer, uint64 Size)
	{
		UploadResourceRequest request{};
		request.UploadType	= EUploadType::Buffer;
		request.Resource	= pBuffer;

		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);

		auto heapProperties = D3D::HeapPropertiesUpload();
		
		D3D12Resource* uploadResource = new D3D12Resource();

		VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadResource->GetHandle())));

		uploadResource->SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);

		void* mapped;
		uploadResource->GetHandle()->Map(0, 0, &mapped);
		memcpy(mapped, pBuffer->GetBufferDesc().Data, Size);
		uploadResource->GetHandle()->Unmap(0, 0);

		request.UploadResource = uploadResource;

		PendingRequests.push_back(request);

	}

	void D3D12UploadContext::UploadTexture(D3D12Texture* pTexture)
	{
		UploadResourceRequest request{};
		request.UploadType = EUploadType::Texture;
		request.Resource = pTexture;

		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(pTexture->Subresource.SlicePitch);

		auto heapProperties = D3D::HeapPropertiesUpload();

		D3D12Resource* uploadResource = new D3D12Resource();

		VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadResource->GetHandle())));

		uploadResource->SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
		uploadResource->SetDebugName("D3D12 Upload Texture Resource");

		request.UploadResource = uploadResource;

		PendingRequests.push_back(request);

	}
} // namespace Luden
