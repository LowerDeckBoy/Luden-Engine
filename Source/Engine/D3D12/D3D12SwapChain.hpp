#pragma once

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include <Platform/Window.hpp>

namespace Luden
{
	class D3D12Device;
	class D3D12CommandQueue;

	enum class DisplayMode
	{
		SDR,
		HDR,
	};

	class D3D12SwapChain
	{
	public:
		D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow);
		// Better solutions?
		D3D12SwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, HWND Handle, uint32 Width, uint32 Height);
		~D3D12SwapChain();

		Ref<IDXGISwapChain4>& GetHandle()
		{
			return m_SwapChain;
		}

		IDXGISwapChain4* GetHandleRaw()
		{
			return m_SwapChain.Get();
		}

		void Resize(uint32 Width, uint32 Height);

		// HDR: DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
		bool IsDisplayHDR();

		void EnableHDR();
		void DisableHDR();

	private:
		void CreateSwapChain(D3D12Device* pDevice, D3D12CommandQueue* pCommandQueue, Platform::Window* pWindow);

		void CreateBackBuffers();

		void SetViewport(uint32 Width, uint32 Height);

		void QueryDisplayInfo();

	private:
		Ref<IDXGISwapChain4>	m_SwapChain;
		D3D12_VIEWPORT			m_Viewport{};
		D3D12_RECT				m_Scissor{};
		DXGI_FORMAT				m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

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
