#include "Renderer.hpp"
#include "D3D12/D3D12Utility.hpp"


namespace Luden
{
	SceneRenderTargets Renderer::SceneTextures = {};

	Renderer::Renderer(D3D12RHI* pD3D12RHI)
		: m_D3D12RHI(pD3D12RHI)
	{
		

		SceneTextures.Scene.Create(m_D3D12RHI, {
			.Usage  = TextureUsageFlag::RenderTarget,
			.pData  = nullptr,
			.Width  = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Width),
			.Height = static_cast<uint32>(m_D3D12RHI->SwapChain->GetSwapChainViewport().Viewport.Height),
			.Format = m_D3D12RHI->SwapChain->GetSwapChainFormat(),
			},
			"Scene Output Render Texture");

	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update()
	{
	}

	void Renderer::Render()
	{
		auto& frame = m_D3D12RHI->Frames.at(BackBufferIndex);
		auto& backbuffer = m_D3D12RHI->SwapChain->BackBuffers.at(BackBufferIndex);

		VERIFY_D3D12_RESULT(frame.GraphicsCommandList->Open());

		frame.GraphicsCommandList->SetDescriptorHeap(m_D3D12RHI->ShaderResourceHeap);
		frame.GraphicsCommandList->SetViewport(&m_D3D12RHI->SwapChain->GetSwapChainViewport());

		frame.GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetCpuStartHandle(), BackBufferIndex, m_D3D12RHI->SwapChain->GetSwapChainDescriptorHeap().GetDescriptorIncrementSize());

		frame.GraphicsCommandList->GetHandleRaw()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		const float clearColor[4] = { 0.5f, 0.2f, 0.7f, 1.0f };
		frame.GraphicsCommandList->GetHandleRaw()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		//VERIFY_D3D12_RESULT(frame.GraphicsCommandList->Close());
		frame.GraphicsCommandList->ResourceTransition(&backbuffer, D3D12_RESOURCE_STATE_PRESENT);

		m_D3D12RHI->GraphicsQueue->Execute({ frame.GraphicsCommandList });

		Present(1);

		m_D3D12RHI->Wait();
		m_D3D12RHI->AdvanceFrame();
	}

	void Renderer::Present(uint32 SyncInterval)
	{
		m_D3D12RHI->SwapChain->Present(SyncInterval);
	}
} // namespace Luden
