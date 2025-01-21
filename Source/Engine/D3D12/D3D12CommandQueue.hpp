#pragma once

#include <Core/RefPtr.hpp>
#include <Core/Types.hpp>
#include <D3D12AgilitySDK/d3d12.h>

namespace Luden
{
	class D3D12Device;

	class D3D12Fence
	{
	public:
		D3D12Fence() = default;
		D3D12Fence(D3D12Device* pDevice);
		~D3D12Fence();

		Ref<ID3D12Fence1> Fence;
		::HANDLE FenceEvent{};		

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

		void Execute();

		void Signal();

		void Wait();

		D3D12_COMMAND_LIST_TYPE GetQueueType() const
		{
			return m_QueueType;
		}

	private:
		Ref<ID3D12CommandQueue> m_CommandQueue;

		D3D12Fence m_Fence;

		D3D12_COMMAND_LIST_TYPE m_QueueType = D3D12_COMMAND_LIST_TYPE_NONE;

	};
} // namespace Luden
