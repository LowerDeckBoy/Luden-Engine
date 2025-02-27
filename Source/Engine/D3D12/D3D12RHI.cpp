#include "D3D12RHI.hpp"
#include "D3D12Utility.hpp"
#include <cassert>

namespace Luden
{
	D3D12RHI::D3D12RHI(Platform::Window* pParentWindow)
		: m_ParentWindow(pParentWindow)
	{
		Adapter = new D3D12Adapter();
		Device	= new D3D12Device(Adapter);

		GraphicsQueue = new D3D12CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		SwapChain = new D3D12SwapChain(Device, GraphicsQueue, m_ParentWindow);

		SceneDepthBuffer = new D3D12DepthBuffer(Device, &SwapChain->GetSwapChainViewport());

		Frames.resize(Config::Get().NumBackBuffers);

		for (auto& frame : Frames)
		{
			frame.GraphicsCommandList = new D3D12CommandList(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		}

		// Frame sync
		{
			FrameSync.GraphicsFence.Create(Device, "D3D12 Frame Sync Graphics Fence");
			FrameSync.Event = ::CreateEvent(nullptr, FALSE, FALSE, L"D3D12 Frame Sync Graphics Fence Event");
			FrameSync.CurrentValues.resize(Config::Get().NumBackBuffers, 0);
			FrameSync.SignaledValues.resize(Config::Get().NumBackBuffers, 0);
			++FrameSync.CurrentValues.at(0);
		}

		bInitialized = true;

		D3D12UploadContext::Create(this);

	}

	D3D12RHI::~D3D12RHI()
	{
		for (auto buffer : Device->Buffers)
		{
			//buffer->Release();
			delete buffer;
		}

		for (auto buffer : Device->ConstantBuffers)
		{
			delete buffer;
		}

		for (auto& frame : Frames)
		{
			delete frame.GraphicsCommandList;
		}

		SAFE_RELEASE(FrameSync.GraphicsFence.Fence);

		delete SceneDepthBuffer;

		delete GraphicsQueue;
		delete SwapChain;
		delete Adapter;
		delete Device;
	}

	void D3D12RHI::Wait()
	{
		GraphicsQueue->Signal(&FrameSync.GraphicsFence, FrameSync.GetCurrentValue());

		if (SUCCEEDED(FrameSync.GraphicsFence.Fence->SetEventOnCompletion(FrameSync.GetCurrentValue(), FrameSync.Event)))
		{
			if (::WaitForSingleObject(FrameSync.Event, 3'000) == WAIT_TIMEOUT)
			{
				LOG_ERROR("GPU Timeout (Wait())");
			}
		}
		else
		{
			LOG_FATAL("[Wait()]: FrameSync.Fence.Fence->SetEventOnCompletion(FrameSync.GetCurrentValue(), FrameSync.Event)");
		}

		FrameSync.SignaledValues.at(BackBufferIndex) = FrameSync.GetCurrentValue();
		FrameSync.CurrentValues.at(BackBufferIndex)++;

	}

	void D3D12RHI::Flush()
	{
		ID3D12Fence* fence = nullptr;
		VERIFY_D3D12_RESULT(Device->LogicalDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		VERIFY_D3D12_RESULT(GraphicsQueue->GetHandle()->Signal(fence, 1));

		HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		VERIFY_D3D12_RESULT(fence->SetEventOnCompletion(1, fenceEvent));
		if (fenceEvent)
		{
			if (::WaitForSingleObject(fenceEvent, 2'000) == WAIT_TIMEOUT)
			{
				LOG_FATAL("GPU Timeout (Flush)");
			}
			::CloseHandle(fenceEvent);
		}

		BackBufferIndex = 0;

		SAFE_DELETE(fence);
	}

	void D3D12RHI::Present(uint32 SyncInterval)
	{
		SwapChain->Present(SyncInterval);

		AdvanceFrame();
	}

	void D3D12RHI::AdvanceFrame()
	{
		const uint64 currentFenceValue = FrameSync.GetCurrentValue();

		GraphicsQueue->Signal(&FrameSync.GraphicsFence, currentFenceValue);

		UpdateBackBufferIndex();

		const uint64& nextFenceValue = FrameSync.GetCurrentValue();

		//if (FrameSync.SignaledValues.at(BackBufferIndex) <= nextFenceValue)
		if (FrameSync.GraphicsFence.Fence->GetCompletedValue() < nextFenceValue)
		{
			if (SUCCEEDED(FrameSync.GraphicsFence.Fence->SetEventOnCompletion(nextFenceValue, FrameSync.Event)))
			{
				if (::WaitForSingleObject(FrameSync.Event, 3'000) == WAIT_TIMEOUT)
				{
					LOG_ERROR("GPU Timeout (AdvanceFrame())");
				}
			}
			else
			{
				LOG_FATAL("[AdvanceFrame()]: FrameSync.Fence.Fence->SetEventOnCompletion(FrameSync.GetCurrentValue(), FrameSync.Event)");
			}
		}

		FrameSync.SignaledValues.at(BackBufferIndex) = currentFenceValue;
		FrameSync.CurrentValues.at(BackBufferIndex) = (currentFenceValue + 1);
		
	}

	uint64 D3D12RHI::QueryAdapterMemory() const
	{
		return Device->ParentAdapter->QueryAdapterMemory();
	}

	void D3D12RHI::CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips, uint32 Count)
	{
		Device->CreateShaderResourceView(pResource, Descriptor, NumMips, Count);
	}

	void D3D12RHI::CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count)
	{
		Device->CreateRenderTargetView(pResource, Descriptor, Count);
	}

	void D3D12RHI::CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
	{
		Device->CreateDepthStencilView(pResource, Descriptor, Format);
	}

	D3D12DepthBuffer D3D12RHI::CreateDepthBuffer(DXGI_FORMAT Format)
	{
		return D3D12DepthBuffer(Device, &SwapChain->GetSwapChainViewport(), Format);
	}

	void FFrameSync::Wait()
	{
		uint64 current = GetCurrentValue();
		auto& fence = GraphicsFence;
		uint64 completed = fence.Fence->GetCompletedValue();
		if (completed < current)
		{
			HRESULT res = fence.Fence->SetEventOnCompletion(current, Event);
			if (SUCCEEDED(res))
			{
				if (::WaitForSingleObject(Event, 2'000) != WAIT_OBJECT_0)
				{
					LOG_FATAL("GPU Timeout!");
				}
			}
			else
			{
				LOG_FATAL("Failed to sync frames!");
				const auto fmt = std::format("completed: {0} < current: {1}", completed, current);
				LOG_FATAL(fmt);
				VERIFY_D3D12_RESULT(res);
				std::exit(-1);
			}
		}
		
	}

	void FFrameSync::Signal(D3D12CommandQueue* pCommandQueue)
	{
		pCommandQueue->Signal(&GraphicsFence, CurrentValues.at(BackBufferIndex));
	}

	void FFrameSync::Signal(D3D12CommandQueue* pCommandQueue, uint64 Value)
	{
		pCommandQueue->Signal(&GraphicsFence, Value);
	}
} // namespace Luden
