#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Utility.hpp"
#include "Config.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	D3D12SwapChain::D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow)
	{
		CreateSwapChain(pDevice, pCommandQueue, pWindow);

		QueryDisplayInfo();

		bIsDisplayHDR = IsDisplayHDR();

		if (bIsDisplayHDR)
		{
			m_SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
		}

		LOG_INFO("Using {0} {1}x{2} display. Bits per color: {3}.",
			/* {0}	*/ (IsDisplayHDR() ? "HDR" : "SDR"),
			/* {1-2}*/ m_DisplayOutputDesc.DesktopCoordinates.right, m_DisplayOutputDesc.DesktopCoordinates.bottom,
			/* {3}	*/ m_DisplayOutputDesc.BitsPerColor);

	}

	D3D12SwapChain::~D3D12SwapChain()
	{
	}

	void D3D12SwapChain::CreateSwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow)
	{
		m_ParentDevice = pDevice;
		m_ParentQueue = pCommandQueue;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.BufferCount	= Config::Get().NumBackBuffers;
		desc.Format			= m_SwapChainFormat;
		desc.Width			= pWindow->Width;
		desc.Height			= pWindow->Height;
		desc.AlphaMode		= DXGI_ALPHA_MODE_IGNORE;
		desc.SampleDesc		= { 1, 0 };
		desc.Flags			= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;


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

		SetViewport(desc.Width, desc.Height);

		m_SwapChainDescriptorHeap.Create(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Config::Get().NumBackBuffers, false);

	}

	void D3D12SwapChain::CreateBackBuffers()
	{
		for (uint32 i = 0; i < Config::Get().NumBackBuffers; ++i)
		{

		}
	}

	void D3D12SwapChain::SetViewport(uint32 Width, uint32 Height)
	{
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width	= static_cast<f32>(Width);
		m_Viewport.Height	= static_cast<f32>(Height);
		m_Viewport.MinDepth = D3D12_MIN_DEPTH;
		m_Viewport.MaxDepth = D3D12_MAX_DEPTH;

		m_Scissor.left		= 0;
		m_Scissor.top		= 0;
		m_Scissor.right		= Width;
		m_Scissor.bottom	= Height;
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
