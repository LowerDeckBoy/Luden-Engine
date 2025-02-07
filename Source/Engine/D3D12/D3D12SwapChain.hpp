#pragma once

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include <Platform/Window.hpp>
#include "RHI/Common.hpp"

namespace Luden
{
	class D3D12Device;
	class D3D12CommandQueue;

	enum class DisplayMode
	{
		SDR,
		HDR,
	};

	class D3D12Viewport
	{
	public:
		D3D12Viewport() = default;
		D3D12Viewport(uint32 Width, uint32 Height)
		{
			SetDimensions(Width, Height);
		}

		void SetDimensions(uint32 Width, uint32 Height)
		{
			Viewport.TopLeftX	= 0.0f;
			Viewport.TopLeftY	= 0.0f;
			Viewport.Width		= static_cast<f32>(Width);
			Viewport.Height		= static_cast<f32>(Height);
			Viewport.MinDepth	= D3D12_MIN_DEPTH;
			Viewport.MaxDepth	= D3D12_MAX_DEPTH;

			Scissor.left		= 0;
			Scissor.top			= 0;
			Scissor.right		= Width;
			Scissor.bottom		= Height;
		}

		D3D12_VIEWPORT Viewport{};
		D3D12_RECT Scissor{};
	};

	class D3D12SwapChain
	{
	public:
		D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow);
		// Better solutions?
		//D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, ::HWND Handle, uint32 Width, uint32 Height);
		~D3D12SwapChain();

		Ref<IDXGISwapChain4>& GetHandle()
		{
			return m_SwapChain;
		}

		IDXGISwapChain4* GetHandleRaw()
		{
			return m_SwapChain.Get();
		}

		void Present(uint32 SyncInterval);

		void Resize(uint32 Width, uint32 Height);

		// Returns true, if current display if HDR capable.
		// SDR:	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
		// HDR:	DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
		bool IsDisplayHDR();

		// TODO:
		//void EnableHDR();
		// TODO:
		//void DisableHDR();

		std::vector<D3D12Resource> BackBuffers;

		DXGI_FORMAT& GetSwapChainFormat()
		{
			return m_SwapChainFormat;
		}

		D3D12Viewport& GetSwapChainViewport()
		{
			return m_SwapChainViewport;
		}

		D3D12DescriptorHeap& GetSwapChainDescriptorHeap()
		{
			return m_SwapChainDescriptorHeap;
		}

	private:
		void CreateSwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow);

		void CreateBackBuffers();

		void SetViewport(uint32 Width, uint32 Height);

		void QueryDisplayInfo();

	private:
		Ref<IDXGISwapChain4>	m_SwapChain;
		DXGI_FORMAT				m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		
		D3D12Viewport m_SwapChainViewport;

		// Owns SwapChain's BackBuffer RenderTargetViews.
		D3D12DescriptorHeap m_SwapChainDescriptorHeap;

		DXGI_COLOR_SPACE_TYPE m_ColorSpace;

		Ref<IDXGIOutput6> m_DisplayOutput{};
		DXGI_OUTPUT_DESC1 m_DisplayOutputDesc{};

		DisplayMode m_DisplayMode;

		bool bIsDisplayHDR;

		D3D12Device* m_ParentDevice;
		D3D12CommandQueue* m_ParentQueue = nullptr;
		// Remove?
		Platform::Window* m_ParentWindow = nullptr;

	};
} // namespace Luden
