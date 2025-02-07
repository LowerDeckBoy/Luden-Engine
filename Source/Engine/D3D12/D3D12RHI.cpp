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

		FrameFence = new D3D12Fence(Device);
		NAME_D3D12_OBJECT(FrameFence->Fence.Get(), "D3D12 Frame Fence");

		ShaderResourceHeap	= new D3D12DescriptorHeap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1'000);
		RenderTargetHeap	= new D3D12DescriptorHeap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
		DepthStencilHeap	= new D3D12DescriptorHeap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64);

		Frames.resize(Config::Get().NumBackBuffers);
		FenceValue = 0;
		FenceEvent = ::CreateEventA(nullptr, FALSE, FALSE, "Frame Fence Event");

		assert(FenceEvent);
		for (auto& frame : Frames)
		{
			frame.GraphicsCommandList = new D3D12CommandList(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

			frame.FenceValue = 0;
		}

		bInitialized = true;
	}

	D3D12RHI::~D3D12RHI()
	{
		for (auto& frame : Frames)
		{
			delete frame.GraphicsCommandList;

			frame.FenceValue = 0;
		}

		delete ShaderResourceHeap;
		delete RenderTargetHeap;
		delete DepthStencilHeap;

		delete FrameFence;

		delete GraphicsQueue;
		delete SwapChain;
		delete Adapter;
		delete Device;
	}

	void D3D12RHI::Wait()
	{
		//GraphicsQueue->Signal(FrameFence, Frames.at(BackBufferIndex).FenceValue);
		//FrameFence->Wait(Frames.at(BackBufferIndex).FenceValue);
		//Frames.at(BackBufferIndex).FenceValue++;
		
		GraphicsQueue->Signal(FrameFence, FenceValue);
		FrameFence->Wait(FenceValue);
		Frames.at(BackBufferIndex).FenceValue = FenceValue;

	}

	void D3D12RHI::Flush()
	{
		for (auto& frame : Frames)
		{
			frame.Wait(FrameFence, FenceEvent);
		}

		ID3D12Fence* fence = nullptr;
		VERIFY_D3D12_RESULT(Device->LogicalDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		VERIFY_D3D12_RESULT(GraphicsQueue->GetHandleRaw()->Signal(fence, 1));

		HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(1, fenceEvent);
		if (fenceEvent)
		{
			::WaitForSingleObject(fenceEvent, 5'000'000);
			::CloseHandle(fenceEvent);
		}

		BackBufferIndex = 0;

		SAFE_DELETE(fence);
	}

	void D3D12RHI::AdvanceFrame()
	{
		uint64& fenceValue = FenceValue;
		++fenceValue;
		Frames.at(BackBufferIndex).FenceValue = fenceValue;
		GraphicsQueue->Signal(FrameFence, fenceValue);
		UpdateBackBufferIndex();
		//GraphicsQueue->Signal(FrameFence, Frames.at(BackBufferIndex)->FenceValue);
		//BackBufferIndex = (BackBufferIndex + 1) % Config::Get().NumBackBuffers;

		/*
		const uint64 frameValue = Frames.at(BackBufferIndex).FenceValue;
		GraphicsQueue->Signal(FrameFence, frameValue);

		UpdateBackBufferIndex();

		if (FrameFence->Fence->GetCompletedValue() < Frames.at(BackBufferIndex).FenceValue)
		{
			FrameFence->Wait(Frames.at(BackBufferIndex).FenceValue);
		}

		Frames.at(BackBufferIndex).FenceValue = frameValue + 1;
		*/
	}

	uint64 D3D12RHI::QueryAdapterMemory() const
	{
		return Device->ParentAdapter->QueryAdapterMemory();
	}

	void D3D12RHI::CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips, uint32 Count)
	{
		const auto& resourceDesc = pResource->GetHandle()->GetDesc1();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = resourceDesc.Format;

		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = NumMips;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.PlaneSlice = 0;

		ShaderResourceHeap->Allocate(Descriptor, Count);

		Device->LogicalDevice->CreateShaderResourceView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);
	}

	void D3D12RHI::CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count)
	{
		const auto& resourceDesc = pResource->GetHandle()->GetDesc1();

		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = resourceDesc.Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		RenderTargetHeap->Allocate(Descriptor, Count);

		Device->LogicalDevice->CreateRenderTargetView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);
	}

	void D3D12RHI::CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
		desc.Format = Format;

		DepthStencilHeap->Allocate(Descriptor);

		Device->LogicalDevice->CreateDepthStencilView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);
	}

	D3D12DepthBuffer D3D12RHI::CreateDepthBuffer(DXGI_FORMAT Format)
	{
		return D3D12DepthBuffer(this, &SwapChain->GetSwapChainViewport(), Format);
	}

	void Frame::Wait(D3D12Fence* pFence) const
	{
		if (pFence->Fence->GetCompletedValue() < FenceValue)
		{
			VERIFY_D3D12_RESULT(pFence->Fence->SetEventOnCompletion(FenceValue, pFence->FenceEvent));
			if (::WaitForSingleObjectEx(pFence->FenceEvent, 2'000'000, FALSE) == WAIT_TIMEOUT)
			{
				LOG_FATAL("GPU TIMEOUT");
			}
		}
	}

	void Frame::Wait(D3D12Fence* pFence, ::HANDLE Event) const
	{
		if (pFence->Fence->GetCompletedValue() < FenceValue)
		{
			VERIFY_D3D12_RESULT(pFence->Fence->SetEventOnCompletion(FenceValue, Event));
			if (::WaitForSingleObjectEx(Event, 2'000'000, TRUE) == WAIT_TIMEOUT)
			{
				LOG_FATAL("GPU TIMEOUT");
			}
		}
	}
} // namespace Luden
