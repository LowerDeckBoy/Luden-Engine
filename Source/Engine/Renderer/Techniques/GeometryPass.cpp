#include "Scene/Scene.hpp"
#include "Asset/ShaderCompiler.hpp"
#include "GeometryPass.hpp"
#include "D3D12/D3D12Utility.hpp"

namespace Luden
{
	GeometryPass::GeometryPass(D3D12RHI* pRHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pRHI)
	{
		Initialize(pRHI, Width, Height);
		CreatePipeline(pShaderCompiler);
	}

	GeometryPass::~GeometryPass()
	{
		Release();
	}

	void GeometryPass::Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height)
	{
		Release();

		DXGI_FORMAT swapChainFormat = pRHI->SwapChain->GetSwapChainFormat();

		BaseColor.Create(pRHI->Device, Width, Height, swapChainFormat, RenderTargetClearColor, "GBuffer BaseColor");
		Normal.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, RenderTargetClearColor, "GBuffer Normal");
		MetallicRoughness.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer MetallicRoughness");
		Emissive.Create(pRHI->Device, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, RenderTargetClearColor, "GBuffer Emissive");

		m_RenderTargetHandles.push_back(&BaseColor.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Normal.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&MetallicRoughness.RenderTargetHandle);
		m_RenderTargetHandles.push_back(&Emissive.RenderTargetHandle);

	}

	void GeometryPass::CreatePipeline(ShaderCompiler* pShaderCompiler)
	{
		Pipeline.Amplification	= pShaderCompiler->CompileAS("../../Shaders/Mesh/GBuffer_MS.hlsl", false);
		Pipeline.Mesh			= pShaderCompiler->CompileMS("../../Shaders/Mesh/GBuffer_MS.hlsl", true);
		Pipeline.Pixel			= pShaderCompiler->CompilePS("../../Shaders/Mesh/GBuffer_MS.hlsl", false);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(m_RHI->Device, &Pipeline.Mesh, PipelineType::Graphics));

		D3D12MeshPipelineStateBuilder builder(m_RHI->Device);
		builder.SetRootSignature(&Pipeline.RootSignature);
		builder.SetAmplificationShader(&Pipeline.Amplification);
		builder.SetMeshShader(&Pipeline.Mesh);
		builder.SetPixelShader(&Pipeline.Pixel);
		builder.EnableDepth(true);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.SetAlphaOpaqueMode(0);
		builder.SetRenderTargetFormats({
			BaseColor.GetFormat(),
			Normal.GetFormat(),
			MetallicRoughness.GetFormat(),
			Emissive.GetFormat()
			});

		VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));
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

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->ResourcesTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Normal,				D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_RENDER_TARGET },
			{ &Emissive,			D3D12_RESOURCE_STATE_RENDER_TARGET } });

		commandList->SetRenderTargets(m_RenderTargetHandles, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTargets(m_RenderTargetHandles, { 0.0f, 0.0f, 0.0f, 0.0f });

		DrawFunction();

		commandList->ResourcesTransition({
			{ &BaseColor,			D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Normal,				D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &MetallicRoughness,	D3D12_RESOURCE_STATE_GENERIC_READ },
			{ &Emissive,			D3D12_RESOURCE_STATE_GENERIC_READ } });
	}
} // namespace Luden
