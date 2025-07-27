#include "D3D12Device.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
	D3D12Pipeline::D3D12Pipeline()
		: m_PipelineType(PipelineType::Graphics)
	{
	}

	D3D12PipelineState::~D3D12PipelineState()
	{
		SAFE_RELEASE(m_PipelineState);
	}

	void D3D12PipelineState::SetName(std::string_view Name) const
	{
		NAME_D3D12_OBJECT(m_PipelineState.Get(), Name);
	}

	D3D12PipelineStateBuilder::D3D12PipelineStateBuilder(D3D12Device* pDevice)
	{
		m_Device = pDevice;

		m_RasterizerDesc	= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_DepthDesc			= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		m_BlendDesc			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		m_CullMode			= D3D12_CULL_MODE_NONE;
		m_FillMode			= D3D12_FILL_MODE_SOLID;

		m_Topology			= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		m_Desc.SampleDesc	= { 1, 0 };
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
		m_Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		m_Desc.DSVFormat = m_DepthFormat;
		Pipeline.GetHandle().AddRef();
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

	void D3D12PipelineStateBuilder::SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology)
	{
		m_Topology = Topology;
	}

	void D3D12PipelineStateBuilder::SetDepthFormat(DXGI_FORMAT Format)
	{
		m_Desc.DSVFormat = Format;
	}

	void D3D12PipelineStateBuilder::SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& Formats)
	{
		m_Desc.NumRenderTargets = static_cast<uint32>(Formats.size());
		
		for (usize i = 0; i < Formats.size(); ++i)
		{
			m_Desc.RTVFormats[i] = Formats.at(i);
		}
	}

	void D3D12PipelineStateBuilder::SetSampleCount(uint32 Sample, uint32 Quality)
	{
		m_Desc.SampleDesc = { Sample, Quality };
	}


	D3D12MeshPipelineStateBuilder::D3D12MeshPipelineStateBuilder(D3D12Device* pDevice)
		: m_Device(pDevice)
	{
		m_RasterizerDesc	= CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT);
		m_DepthDesc			= CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
		m_BlendDesc			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		m_Desc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
		m_Desc.SampleMask = 0xFFFFFFFF;
		m_Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

	HRESULT D3D12MeshPipelineStateBuilder::Build(D3D12PipelineState& Pipeline)
	{
		m_Desc.NodeMask				= m_Device->NodeMask;
		m_Desc.RasterizerState		= m_RasterizerDesc;
		m_Desc.DepthStencilState	= m_DepthDesc;
		m_Desc.BlendState			= m_BlendDesc;
		m_Desc.DSVFormat			= DXGI_FORMAT_D32_FLOAT;
		
		//m_Desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		m_Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(m_Desc);

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
		streamDesc.SizeInBytes = sizeof(psoStream);
		streamDesc.pPipelineStateSubobjectStream = &psoStream;
		Pipeline.GetHandle().AddRef();
		return m_Device->LogicalDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&Pipeline.GetHandle()));
	}

	void D3D12MeshPipelineStateBuilder::SetAmplificationShader(D3D12Shader* pShader)
	{
		m_Desc.AS = pShader->Bytecode();
	}

	void D3D12MeshPipelineStateBuilder::SetMeshShader(D3D12Shader* pShader)
	{
		m_Desc.MS = pShader->Bytecode();
	}

	void D3D12MeshPipelineStateBuilder::SetPixelShader(D3D12Shader* pShader)
	{
		m_Desc.PS = pShader->Bytecode();
	}

	void D3D12MeshPipelineStateBuilder::SetRootSignature(D3D12RootSignature* pRootSignature)
	{
		m_Desc.pRootSignature = pRootSignature->GetHandleRaw();
	}

	void D3D12MeshPipelineStateBuilder::EnableDepth(bool bEnable)
	{
		m_DepthDesc.DepthEnable = bEnable;
		m_RasterizerDesc.DepthClipEnable = bEnable;
	}

	void D3D12MeshPipelineStateBuilder::SetCullMode(D3D12_CULL_MODE CullMode)
	{
		m_RasterizerDesc.CullMode = CullMode;
	}

	void D3D12MeshPipelineStateBuilder::SetFillMode(D3D12_FILL_MODE FillMode)
	{
		m_RasterizerDesc.FillMode = FillMode;
	}

	void D3D12MeshPipelineStateBuilder::SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology)
	{
		m_Desc.PrimitiveTopologyType = Topology;
	}

	void D3D12MeshPipelineStateBuilder::SetDepthFormat(DXGI_FORMAT Format)
	{
		m_Desc.DSVFormat = Format;
	}

	void D3D12MeshPipelineStateBuilder::SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& Formats)
	{
		m_Desc.NumRenderTargets = static_cast<uint32>(Formats.size());

		assert(m_Desc.NumRenderTargets > 0);

		for (usize i = 0; i < Formats.size(); ++i)
		{
			m_Desc.RTVFormats[i] = Formats.at(i);
		}
	}

	void D3D12MeshPipelineStateBuilder::SetAlphaModeOpaque(uint32 RenderTargetIndex)
	{
		D3D12_RENDER_TARGET_BLEND_DESC desc{};
		desc.BlendEnable			= false;
		desc.LogicOpEnable			= false;
		desc.SrcBlend				= D3D12_BLEND_ONE;
		desc.DestBlend				= D3D12_BLEND_ZERO;
		desc.SrcBlendAlpha			= D3D12_BLEND_ONE;
		desc.DestBlendAlpha			= D3D12_BLEND_ZERO;
		desc.BlendOpAlpha			= D3D12_BLEND_OP_ADD;
		desc.BlendOp				= D3D12_BLEND_OP_ADD;
		desc.LogicOp				= D3D12_LOGIC_OP_NOOP;
		desc.RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;

		m_BlendDesc.AlphaToCoverageEnable			= false;
		m_BlendDesc.IndependentBlendEnable			= false;
		m_BlendDesc.RenderTarget[RenderTargetIndex] = desc;
	}

	void D3D12MeshPipelineStateBuilder::SetAlphaModeBlend(uint32 RenderTargetIndex)
	{
		D3D12_RENDER_TARGET_BLEND_DESC desc{};
		desc.BlendEnable			= true;
		desc.LogicOpEnable			= false;
		desc.SrcBlend				= D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend				= D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp				= D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha			= D3D12_BLEND_ONE;
		desc.DestBlendAlpha			= D3D12_BLEND_ONE;
		desc.BlendOpAlpha			= D3D12_BLEND_OP_ADD;
		desc.LogicOp				= D3D12_LOGIC_OP_NOOP;
		desc.RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;

		m_BlendDesc.AlphaToCoverageEnable			= false;
		m_BlendDesc.IndependentBlendEnable			= false;
		m_BlendDesc.RenderTarget[RenderTargetIndex] = desc;
	}

	void D3D12MeshPipelineStateBuilder::SetAlphaModeAdditive(uint32 RenderTargetIndex)
	{
		D3D12_RENDER_TARGET_BLEND_DESC desc{};
		desc.BlendEnable			= true;
		desc.LogicOpEnable			= false;
		desc.SrcBlend				= D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend				= D3D12_BLEND_ONE;
		desc.SrcBlendAlpha			= D3D12_BLEND_ONE;
		desc.DestBlendAlpha			= D3D12_BLEND_ZERO;
		desc.BlendOp				= D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha			= D3D12_BLEND_OP_ADD;
		desc.LogicOp				= D3D12_LOGIC_OP_NOOP;
		desc.RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;
		
		m_BlendDesc.AlphaToCoverageEnable			= false;
		m_BlendDesc.IndependentBlendEnable			= false;
		m_BlendDesc.RenderTarget[RenderTargetIndex] = desc;
	}

	void D3D12MeshPipelineStateBuilder::SetDefaultDepthDesc()
	{
		m_DepthDesc = CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
	}

	void D3D12MeshPipelineStateBuilder::SetAlphaBlendDepthDesc()
	{
		m_DepthDesc.DepthEnable						= true;
		m_DepthDesc.DepthFunc						= D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_DepthDesc.DepthWriteMask					= D3D12_DEPTH_WRITE_MASK_ALL;
		m_DepthDesc.DepthBoundsTestEnable			= false;
		m_DepthDesc.StencilEnable					= false;
		
		m_DepthDesc.FrontFace.StencilFailOp			= D3D12_STENCIL_OP_KEEP;
		m_DepthDesc.FrontFace.StencilDepthFailOp	= D3D12_STENCIL_OP_INCR;
		m_DepthDesc.FrontFace.StencilPassOp			= D3D12_STENCIL_OP_KEEP;
		m_DepthDesc.FrontFace.StencilFunc			= D3D12_COMPARISON_FUNC_ALWAYS;
		m_DepthDesc.FrontFace.StencilReadMask		= D3D12_DEFAULT_STENCIL_READ_MASK;
		m_DepthDesc.FrontFace.StencilWriteMask		= D3D12_DEFAULT_STENCIL_WRITE_MASK;

		m_DepthDesc.BackFace.StencilFailOp			= D3D12_STENCIL_OP_KEEP;
		m_DepthDesc.BackFace.StencilDepthFailOp		= D3D12_STENCIL_OP_DECR;
		m_DepthDesc.BackFace.StencilPassOp			= D3D12_STENCIL_OP_KEEP;
		m_DepthDesc.BackFace.StencilFunc			= D3D12_COMPARISON_FUNC_ALWAYS;
		m_DepthDesc.BackFace.StencilReadMask		= D3D12_DEFAULT_STENCIL_READ_MASK;
		m_DepthDesc.BackFace.StencilWriteMask		= D3D12_DEFAULT_STENCIL_WRITE_MASK;
	}

	D3D12ComputePipelineStateBuilder::D3D12ComputePipelineStateBuilder(D3D12Device* pDevice)
		: m_Device(pDevice)
	{

	}

	HRESULT D3D12ComputePipelineStateBuilder::Build(D3D12Device* pDevice, D3D12Pipeline& OutPipeline)
	{
		m_Desc.NodeMask = pDevice->NodeMask;
		
		return pDevice->LogicalDevice->CreateComputePipelineState(&m_Desc, IID_PPV_ARGS(&OutPipeline.PipelineState.GetHandle()));
	}

	void D3D12ComputePipelineStateBuilder::SetRootSignature(D3D12RootSignature* pRootSignature)
	{
		m_Desc.pRootSignature = pRootSignature->GetHandleRaw();
	}

	void D3D12ComputePipelineStateBuilder::SetComputeShader(D3D12Shader* pShader)
	{
		m_Desc.CS = pShader->Bytecode();
	}

} // namespace Luden
