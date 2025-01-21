#pragma once

#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12SwapChain.hpp"

#include <Platform/Window.hpp>

namespace Luden
{
	class D3D12RHI
	{
	public:
		D3D12RHI(Platform::Window* pParentWindow);
		~D3D12RHI();

		D3D12Adapter* Adapter;
		D3D12Device* Device;

		D3D12CommandQueue* GraphicsQueue;

		D3D12SwapChain* SwapChain;


	private:

		Platform::Window* m_ParentWindow;

		bool bInitialized = false;

	};

} // namespace Luden
