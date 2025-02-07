#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
	void D3D12PipelineState::SetName(std::string_view Name)
	{
		NAME_D3D12_OBJECT(m_PipelineState.Get(), Name);
	}

	D3D12PipelineStateBuilder::D3D12PipelineStateBuilder(D3D12Device* pDevice)
	{
		m_Device = pDevice;

		m_RasterizerDesc	= CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT);
		m_DepthDesc			= CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
		m_BlendDesc			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		m_CullMode = D3D12_CULL_MODE_NONE;
		m_FillMode = D3D12_FILL_MODE_SOLID;

		m_Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		m_Desc.SampleDesc = { 1, 0 };
	}

	HRESULT D3D12PipelineStateBuilder::Build(D3D12PipelineState& Pipeline)
	{
		m_Desc.NodeMask = m_Device->NodeMask;

		m_Desc.RasterizerState = m_RasterizerDesc;
		m_Desc.RasterizerState.CullMode = m_CullMode;
		m_Desc.RasterizerState.FillMode = m_FillMode;

		m_Desc.DepthStencilState = m_DepthDesc;
		m_Desc.BlendState = m_BlendDesc;
		m_Desc.SampleMask = UINT_MAX;

		m_Desc.NumRenderTargets = static_cast<uint32>(m_RenderTargetFormats.size());

		for (usize i = 0; i < m_RenderTargetFormats.size(); ++i)
		{
			m_Desc.RTVFormats[i] = m_RenderTargetFormats.at(i);
		}

		return m_Device->LogicalDevice->CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&Pipeline.GetHandle()));
	}

	void D3D12PipelineStateBuilder::SetRootSignature(D3D12RootSignature* pRootSignature)
	{
		m_Desc.pRootSignature = pRootSignature->GetHandleRaw();
	}

	void D3D12PipelineStateBuilder::SetVertexShader(D3D12Shader* pShader)
	{
		m_Desc.VS = pShader->Bytecode();
	}

	void D3D12PipelineStateBuilder::SetPixelShader(D3D12Shader* pShader)
	{
		m_Desc.PS = pShader->Bytecode();
	}

	void D3D12PipelineStateBuilder::EnableDepth(bool bEnable)
	{
		m_DepthDesc.DepthEnable = bEnable;
		m_RasterizerDesc.DepthClipEnable = bEnable;
	}

	void D3D12PipelineStateBuilder::SetCullMode(D3D12_CULL_MODE CullMode)
	{
		m_CullMode = CullMode;
	}

	void D3D12PipelineStateBuilder::SetFillMode(D3D12_FILL_MODE FillMode)
	{
		m_FillMode = FillMode;
	}

	void D3D12PipelineStateBuilder::SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
	{
		m_Topology = Topology;
	}

	void D3D12PipelineStateBuilder::SetDepthFormat(DXGI_FORMAT Format)
	{
		m_Desc.DSVFormat = Format;
	}

	void D3D12PipelineStateBuilder::SetSampleCount(uint32 Sample, uint32 Quality)
	{
		m_Desc.SampleDesc = { Sample, Quality };
	}


	HRESULT D3D12MeshPipelineState::Build(D3D12PipelineState& /* Pipeline */)
	{
		

		return E_NOTIMPL;
	}


} // namespace Luden
