#pragma once

#include <Core/RefPtr.hpp>
#include <Core/Types.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <vector>

namespace Luden
{
	class D3D12Device;
	class D3D12CommandList;

	class D3D12Fence
	{
	public:
		D3D12Fence() = default;
		D3D12Fence(D3D12Device* pDevice);
		~D3D12Fence();

		Ref<ID3D12Fence1> Fence;
		::HANDLE FenceEvent{};
		
		//void Signal(uint64 ValueToSignal) const;
		void Wait(uint64 Value);

		uint64 CurrentValue = 0;
		uint64 LastSignaledValue = 0;
	};

	class D3D12CommandQueue
	{
	public:
		D3D12CommandQueue(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE QueueType);
		~D3D12CommandQueue();

		Ref<ID3D12CommandQueue>& GetHandle()
		{
			return m_CommandQueue;
		}

		ID3D12CommandQueue* GetHandleRaw()
		{
			return m_CommandQueue.Get();
		}

		// Close (if not closed already) and Execute each passed Command Lists.
		void Execute(const std::vector<D3D12CommandList*>& CommandLists);

		void Signal(D3D12Fence* pFence, uint64 ValueToSignal);
		// Signal this Queue Fence.
		void Signal();

		void Wait(D3D12Fence* pFence, uint64 Value);
		// Wait for this Queue Fence.
		void Wait();

		D3D12_COMMAND_LIST_TYPE GetQueueType() const
		{
			return m_QueueType;
		}

		D3D12Fence Fence;

	private:
		Ref<ID3D12CommandQueue> m_CommandQueue;

		D3D12_COMMAND_LIST_TYPE m_QueueType = D3D12_COMMAND_LIST_TYPE_NONE;

	};
} // namespace Luden
