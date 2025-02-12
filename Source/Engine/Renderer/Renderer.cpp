#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	SceneRenderTargets Renderer::SceneTextures = {};

	Renderer::Renderer(Platform::Window* pParentWindow, D3D12RHI* pD3D12RHI)
		: m_ParentWindow(pParentWindow),
		m_D3D12RHI(pD3D12RHI)
	{
		SceneTextures.Scene.Create(m_D3D12RHI, {
			.Usage  = TextureUsageFlag::RenderTarget,
			.pData  = nullptr,
			.Width  = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			.Height = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			.Format = m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			},
			"Scene Output Render Texture");

		m_D3D12RHI->Wait();
	}

	Renderer::~Renderer()
	{
		SceneTextures.Scene.Release();
	}

	void Renderer::BeginFrame()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		
		VERIFY_D3D12_RESULT(frame->GraphicsCommandList->Open());

		frame->GraphicsCommandList->SetDescriptorHeap(m_D3D12RHI->ShaderResourceHeap);
		frame->GraphicsCommandList->SetViewport(&m_D3D12RHI->SwapChain->GetSwapChainViewport());

	}

	void Renderer::EndFrame()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_PRESENT);

		m_D3D12RHI->GraphicsQueue->Execute({ frame->GraphicsCommandList });
		//m_D3D12RHI->Wait();

		// Leaving it for now, as perhaps might be needed later.
		VERIFY_D3D12_RESULT(frame->GraphicsCommandList->GetHandle()->Reset(frame->GraphicsCommandList->GetAllocator(), nullptr));
		m_D3D12RHI->Wait();
		VERIFY_D3D12_RESULT(frame->GraphicsCommandList->GetHandle()->Close());
		
	}

	void Renderer::Update()
	{
	}

	void Renderer::Render()
	{
		auto* frame = &m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());

		frame->GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		constexpr float clearColor[4] = { 0.5f, 0.2f, 0.7f, 1.0f };
		frame->GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_COPY_DEST);
		frame->GraphicsCommandList->CopyResource(&backbuffer, &SceneTextures.Scene);
		frame->GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		frame->GraphicsCommandList->ResourceTransition(&SceneTextures.Scene, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

	void Renderer::Present(uint32 SyncInterval)
	{
		m_D3D12RHI->Present(SyncInterval);
		
	}

	void Renderer::Resize()
	{
		m_D3D12RHI->Wait();
		m_D3D12RHI->Flush();

		//BackBufferIndex = m_D3D12RHI->SwapChain->GetHandle()->GetCurrentBackBufferIndex();

		for (uint32 i = 0; i < Config::Get().NumBackBuffers; ++i)
		{
			m_D3D12RHI->FrameSync.CurrentValues.at(i) = m_D3D12RHI->FrameSync.CurrentValues.at(BackBufferIndex);

			m_D3D12RHI->Frames.at(i).GraphicsCommandList->Open();

		}

		m_ParentWindow->Resize();
		m_D3D12RHI->SwapChain->Resize(m_ParentWindow->Width, m_ParentWindow->Height);

		SceneTextures.Scene.Resize(m_ParentWindow->Width, m_ParentWindow->Height);
		
		std::vector<D3D12CommandList*> lists;
		for (auto& frame : m_D3D12RHI->Frames)
		{
			lists.emplace_back(frame.GraphicsCommandList);
		}

		m_D3D12RHI->GraphicsQueue->Execute(lists);
		m_D3D12RHI->Wait();

	}

} // namespace Luden
