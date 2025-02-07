#include "D3D12Device.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>
#include <cassert>

namespace Luden
{
	D3D12Fence::D3D12Fence(D3D12Device* pDevice)
	{
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
		FenceEvent = ::CreateEventA(nullptr, false, false, nullptr);
		assert(FenceEvent);
	}

	D3D12Fence::~D3D12Fence()
	{
		SAFE_RELEASE(Fence);
		::CloseHandle(FenceEvent);
	}

	//void D3D12Fence::Signal(uint64 ValueToSignal) const
	//{
	//	VERIFY_D3D12_RESULT(Fence->Signal(ValueToSignal));
	//}

	void D3D12Fence::Wait(uint64 Value)
	{
		auto completed = Fence->GetCompletedValue();
		if (completed < Value)
		{
			::HANDLE event = ::CreateEventA(nullptr, false, false, "Fence Wait Event");
			//VERIFY_D3D12_RESULT(Fence->SetEventOnCompletion(Value, FenceEvent));

			if (event)
			{
				VERIFY_D3D12_RESULT(Fence->SetEventOnCompletion(Value, event));

				if (::WaitForSingleObject(event, 5'000'000) == WAIT_TIMEOUT)
				{
					LOG_FATAL("GPU TIMEOUT!");
				}
				::CloseHandle(event);
			}
		}
		
	}

	D3D12CommandQueue::D3D12CommandQueue(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE QueueType)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask	= pDevice->NodeMask;
		desc.Type		= QueueType;
		desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;

		m_QueueType = QueueType;

		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

		switch (QueueType)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			NAME_D3D12_OBJECT(m_CommandQueue.Get(), "D3D12 Direct Command Queue");
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			NAME_D3D12_OBJECT(m_CommandQueue.Get(), "D3D12 Compute Command Queue");
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			NAME_D3D12_OBJECT(m_CommandQueue.Get(), "D3D12 Copy Command Queue");
			break;
		}

		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateFence(Fence.CurrentValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence.Fence)));
		Fence.FenceEvent = ::CreateEventA(nullptr, false, false, nullptr);
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		SAFE_RELEASE(m_CommandQueue);
	}

	void D3D12CommandQueue::Execute(const std::vector<D3D12CommandList*>& CommandLists)
	{
		std::vector<ID3D12CommandList*> commandLists;

		for (auto& list : CommandLists)
		{	
			if (list->IsOpen())
			{
				VERIFY_D3D12_RESULT(list->Close());
			}

			commandLists.push_back(list->GetHandleRaw());
		}

		m_CommandQueue->ExecuteCommandLists(static_cast<uint32>(commandLists.size()), commandLists.data());

	}

	void D3D12CommandQueue::Signal(D3D12Fence* pFence, uint64 ValueToSignal)
	{
		VERIFY_D3D12_RESULT(m_CommandQueue->Signal(pFence->Fence.Get(), ValueToSignal));
	}

	void D3D12CommandQueue::Signal()
	{
		VERIFY_D3D12_RESULT(m_CommandQueue->Signal(Fence.Fence.Get(), Fence.CurrentValue));
	}

	void D3D12CommandQueue::Wait(D3D12Fence* pFence, uint64 Value)
	{
		VERIFY_D3D12_RESULT(m_CommandQueue->Wait(pFence->Fence.Get(), Value));
	}

	void D3D12CommandQueue::Wait()
	{
		VERIFY_D3D12_RESULT(m_CommandQueue->Wait(Fence.Fence.Get(), Fence.CurrentValue));
	}

} // namespace Luden
