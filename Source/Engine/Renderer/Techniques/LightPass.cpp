#include "Asset/ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"
#include "LightPass.hpp"

namespace Luden
{
	LightPass::LightPass(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{
		Initialize(pD3D12RHI, pShaderCompiler, pGeometryPass, Width, Height);
	}

	LightPass::~LightPass()
	{
		Release();
	}

	void LightPass::Initialize(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height)
	{
		m_RHI = pD3D12RHI;
		m_GeometryPass = pGeometryPass;

		RenderTexture.Create(m_RHI->Device, Width, Height, DXGI_FORMAT_R32G32B32A32_FLOAT, DefaultClearColor, "Light Pass Render Target");

		Pipeline.Vertex = pShaderCompiler->CompileVS("../../Shaders/Deferred/Deferred.hlsl", false);
		Pipeline.Pixel = pShaderCompiler->CompilePS("../../Shaders/Deferred/Deferred.hlsl", true);

		VERIFY_D3D12_RESULT(Pipeline.RootSignature.BuildFromShader(pD3D12RHI->Device, &Pipeline.Pixel, PipelineType::Graphics));

		D3D12PipelineStateBuilder builder(pD3D12RHI->Device);
		builder.EnableDepth(false);
		builder.SetCullMode(D3D12_CULL_MODE_NONE);
		builder.SetVertexShader(&Pipeline.Vertex);
		builder.SetPixelShader(&Pipeline.Pixel);
		builder.SetRenderTargetFormats({ RenderTexture.GetFormat() });

		VERIFY_D3D12_RESULT(builder.Build(Pipeline.PipelineState));

	}

	void LightPass::Render(Scene* pScene, Frame& CurrentFrame)
	{
		auto commandList = CurrentFrame.GraphicsCommandList;

		commandList->SetRootSignature(&Pipeline.RootSignature);
		commandList->SetPipelineState(&Pipeline.PipelineState);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandList->SetRenderTarget(RenderTexture.RenderTargetHandle, m_RHI->SceneDepthBuffer->DepthStencilHandle);
		commandList->ClearRenderTarget(RenderTexture.RenderTargetHandle, DefaultClearColor);

		// Push Constants here

		// Draw screen space quad.
		commandList->Draw(4);

		commandList->ResourceTransition(&RenderTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

	void LightPass::Resize(uint32 Width, uint32 Height)
	{
		RenderTexture.Resize(Width, Height);
	}

	void LightPass::Release()
	{
		RenderTexture.Release();
	}

} // namespace Luden
