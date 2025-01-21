#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12Utility.hpp"


namespace Luden
{
	D3D12Fence::D3D12Fence(D3D12Device* pDevice)
	{
		VERIFY_D3D12_RESULT(pDevice->Device->CreateFence(CurrentValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
		FenceEvent = ::CreateEventA(nullptr, false, false, nullptr);
	}

	D3D12Fence::~D3D12Fence()
	{
		SAFE_RELEASE(Fence);
		::CloseHandle(FenceEvent);
	}

	D3D12CommandQueue::D3D12CommandQueue(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE QueueType)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask = pDevice->NodeMask;
		desc.Type = QueueType;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		m_QueueType = QueueType;

		VERIFY_D3D12_RESULT(pDevice->Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
	}

} // namespace Luden
