#include "Scene/Scene.hpp"
#include "GeometryPass.hpp"

namespace Luden
{
	GeometryPass::GeometryPass(D3D12RHI* pRHI, uint32 Width, uint32 Height)
		: RenderPass(pRHI)
	{
		Initialize(pRHI, Width, Height);
	}

	GeometryPass::~GeometryPass()
	{
		Release();
	}

	void GeometryPass::Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height)
	{
		Release();

		BaseColor.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer BaseColor");
		Normal.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, RenderTargetClearColor, "GBuffer Normal");
		MetallicRoughness.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer MetallicRoughness");
		Emissive.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer Emissive");

		m_RenderTargetHandles.push_back(&BaseColor.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Normal.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MetallicRoughness.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Emissive.RenderTargetHandle);
	}

	void GeometryPass::Release()
	{
		BaseColor.Release();
		Normal.Release();
		MetallicRoughness.Release();
		Emissive.Release();

		m_RenderTargetHandles.clear();
		m_RenderTargetHandles.shrink_to_fit();

	}

	void GeometryPass::Resize(uint32 Width, uint32 Height)
	{
		BaseColor.Resize(Width, Height);
		Normal.Resize(Width, Height);
		MetallicRoughness.Resize(Width, Height);
		Emissive.Resize(Width, Height);
	}
	
	void GeometryPass::Render(Frame& CurrentFrame, std::function<void()> const& DrawFunction)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;
		//auto device = m_RHI->Device;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		//commandList->ResourceTransition(Depth, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(&BaseColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(&Normal, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(&MetallicRoughness, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceTransition(&Emissive, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//commandList->ResourceTransition(WorldPosition, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, { 0.0f, 0.0f, 0.0f, 0.0f });

		DrawFunction();

		commandList->ResourceTransition(&BaseColor, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(&Normal, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(&MetallicRoughness, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceTransition(&Emissive, D3D12_RESOURCE_STATE_GENERIC_READ);

	}
} // namespace Luden
