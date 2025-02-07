#pragma once

#include "Config.hpp"

#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Shader.hpp"
#include "D3D12PipelineState.hpp"

#include <Platform/Window.hpp>
#include <Core/Logger.hpp>

namespace Luden
{
	// One per BackBuffer.
	struct Frame
	{
		D3D12CommandList* GraphicsCommandList;

		uint64 FenceValue = 0;

		// https://www.youtube.com/watch?v=TCxsR38yBRs&list=PLU2nPsAdxKWQw1qBS9YdFi9hUMazppjV7&index=7&t=9s
		void Wait(D3D12Fence* pFence) const;
		void Wait(D3D12Fence* pFence, ::HANDLE Event) const;
	};

	class D3D12RHI
	{
	public:
		D3D12RHI(Platform::Window* pParentWindow);
		~D3D12RHI();

		D3D12Adapter* Adapter;
		D3D12Device* Device;

		D3D12CommandQueue* GraphicsQueue;

		D3D12SwapChain* SwapChain;

		D3D12DescriptorHeap* ShaderResourceHeap;
		D3D12DescriptorHeap* RenderTargetHeap;
		D3D12DescriptorHeap* DepthStencilHeap;

		INLINE void UpdateBackBufferIndex()
		{
			BackBufferIndex = (BackBufferIndex + 1) % Config::Get().NumBackBuffers;
		}

		std::vector<Frame> Frames;

		D3D12Fence* FrameFence;
		uint64 FenceValue = 0;
		::HANDLE FenceEvent;

		// Wait for Device to finish it's work.
		void Wait();
		void Flush();

		uint64 QueryAdapterMemory() const;

		void AdvanceFrame();
		
		void CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips = 1, uint32 Count = 1);
		void CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count = 1);
		void CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		//uint32 CreateBuffer();
		//uint32 CreateTexture();

		D3D12DepthBuffer CreateDepthBuffer(DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

	private:
		Platform::Window* m_ParentWindow;

		bool bInitialized = false;

		//std::vector<D3D12Buffer> Buffers;
		//std::vector<D3D12Texture> Textures;

	};

} // namespace Luden
