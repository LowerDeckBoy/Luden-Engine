#pragma once

#include "Config.hpp"

#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12CommandSignature.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Shader.hpp"
#include "D3D12PipelineState.hpp"

#include <Platform/Window.hpp>
#include <Core/Logger.hpp>

// Test
#include "D3D12UploadContext.hpp"

namespace Luden
{
	// One per BackBuffer.
	struct Frame
	{
		D3D12CommandList* GraphicsCommandList;
		//D3D12CommandList* ComputeCommandList;

	};

	// Gathers objects and values for frame buffering syncing.
	struct FFrameSync
	{
		D3D12Fence	GraphicsFence;
		::HANDLE	Event;

		std::vector<uint64> CurrentValues;
		std::vector<uint64> SignaledValues;

		// Get fence value for current BackBufferIndex.
		uint64& GetCurrentValue()
		{
			return CurrentValues.at(BackBufferIndex);
		}

		uint64& GetLastSignaledValue()
		{
			return SignaledValues.at(BackBufferIndex);
		}

		void Wait();

		void Signal(D3D12CommandQueue* pCommandQueue);
		void Signal(D3D12CommandQueue* pCommandQueue, uint64 Value);
		
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

		D3D12DepthBuffer* SceneDepthBuffer;

		INLINE void UpdateBackBufferIndex()
		{
			BackBufferIndex = (BackBufferIndex + 1) % Config::Get().NumBackBuffers;
		}

		std::vector<Frame> Frames;

		FFrameSync FrameSync;

		// Wait for Device to finish it's work.
		void Wait();
		void Flush();

		uint64 QueryAdapterMemory() const;

		void Present(uint32 SyncInterval);
		void AdvanceFrame();
		
		void CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips = 1, uint32 Count = 1);
		void CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count = 1);
		void CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		D3D12DepthBuffer CreateDepthBuffer(DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		D3D12CommandSignature* MeshCommandSignature;

	private:
		Platform::Window* m_ParentWindow;

		bool bInitialized = false;

	};

} // namespace Luden
