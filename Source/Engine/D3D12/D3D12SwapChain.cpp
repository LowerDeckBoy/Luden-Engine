#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Utility.hpp"
#include "Config.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	uint32 BackBufferIndex = 0;

	D3D12SwapChain::D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow)
	{
		BackBuffers.resize(Config::Get().NumBackBuffers);

		m_ParentDevice = pDevice;
		m_ParentQueue = pCommandQueue;

		m_ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

		QueryDisplayInfo();

		bIsDisplayHDR = IsDisplayHDR();

		if (bIsDisplayHDR)
		{
			m_SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
			m_ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
		}

		CreateSwapChain(pDevice, pCommandQueue, pWindow);
		
		LOG_INFO("Using {0} {1}x{2} display. Bits per color: {3}.",
			/* {0}	*/ (bIsDisplayHDR ? "HDR" : "SDR"),
			/* {1-2}*/ m_DisplayOutputDesc.DesktopCoordinates.right, m_DisplayOutputDesc.DesktopCoordinates.bottom,
			/* {3}	*/ m_DisplayOutputDesc.BitsPerColor);

	}

	D3D12SwapChain::~D3D12SwapChain()
	{
		SAFE_RELEASE(m_SwapChain);
	}

	void D3D12SwapChain::CreateSwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow)
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.BufferCount	= Config::Get().NumBackBuffers;
		desc.Format			= m_SwapChainFormat;
		desc.Width			= pWindow->Width;
		desc.Height			= pWindow->Height;
		desc.AlphaMode		= DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Scaling		= DXGI_SCALING_NONE;
		desc.Stereo			= FALSE;
		desc.SampleDesc		= { 1, 0 };
		desc.Flags			= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		Ref<IDXGISwapChain1> swapchain;
		VERIFY_D3D12_RESULT(pDevice->ParentAdapter->Factory->CreateSwapChainForHwnd(
			pCommandQueue->GetHandleRaw(),
			pWindow->Handle,
			&desc,
			nullptr,
			nullptr,
			swapchain.GetAddressOf()
		));

		swapchain.As(m_SwapChain);
		SAFE_RELEASE(swapchain);

		VERIFY_D3D12_RESULT(m_SwapChain->SetMaximumFrameLatency(Config::Get().NumBackBuffers));
		VERIFY_D3D12_RESULT(pDevice->ParentAdapter->Factory->MakeWindowAssociation(pWindow->Handle, DXGI_MWA_NO_ALT_ENTER));

		m_SwapChainViewport.SetDimensions(desc.Width, desc.Height);

		m_SwapChainDescriptorHeap.Create(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Config::Get().NumBackBuffers, false);
		CreateBackBuffers();

		BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12SwapChain::CreateBackBuffers()
	{
		m_SwapChainDescriptorHeap.Reset();

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_SwapChainDescriptorHeap.GetCpuStartHandle());

		for (uint32 i = 0; i < Config::Get().NumBackBuffers; ++i)
		{
			VERIFY_D3D12_RESULT(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffers.at(i).GetHandle())));

			m_ParentDevice->LogicalDevice->CreateRenderTargetView(BackBuffers.at(i).GetHandleRaw(), nullptr, rtvHandle);

			BackBuffers.at(i).SetDebugName(std::format("BackBuffer #{}", std::to_string(i)));

			rtvHandle.Offset(1, m_SwapChainDescriptorHeap.GetDescriptorIncrementSize());

			BackBuffers.at(i).SetResourceState(D3D12_RESOURCE_STATE_PRESENT);
		}
	}

	void D3D12SwapChain::Present(uint32 SyncInterval)
	{
		//VERIFY_D3D12_RESULT();
		HRESULT present = m_SwapChain->Present(SyncInterval, (SyncInterval == 0) ? DXGI_PRESENT_ALLOW_TEARING : 0);
		if (present == DXGI_ERROR_DEVICE_REMOVED)
		{
			VERIFY_D3D12_RESULT(m_ParentDevice->LogicalDevice->GetDeviceRemovedReason());
		}
		else
		{
			VERIFY_D3D12_RESULT(present);
		}
		//BackBufferIndex = (BackBufferIndex + 1) % Config::Get().NumBackBuffers;
	}

	void D3D12SwapChain::Resize(uint32 Width, uint32 Height)
	{
		for (auto& backbuffer : BackBuffers)
		{
			backbuffer.Release();
		}

		m_SwapChainViewport.SetDimensions(Width, Height);

		VERIFY_D3D12_RESULT(m_SwapChain->ResizeBuffers(Config::Get().NumBackBuffers, Width, Height, m_SwapChainFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

		CreateBackBuffers();

		//BackBufferIndex = 0;
	}

	bool D3D12SwapChain::IsDisplayHDR()
	{
		// Refresh display information.
		QueryDisplayInfo();

		return m_DisplayOutputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
	}
	
	void D3D12SwapChain::QueryDisplayInfo()
	{
		SAFE_RELEASE(m_DisplayOutput);

		Ref<IDXGIOutput> output;
		VERIFY_D3D12_RESULT(m_ParentDevice->ParentAdapter->Adapter->EnumOutputs(0, &output), "Failed to enumerate DXGI Outputs!");
		
		output.As(m_DisplayOutput);
		VERIFY_D3D12_RESULT(m_DisplayOutput->GetDesc1(&m_DisplayOutputDesc));
	}

} // namespace Luden
