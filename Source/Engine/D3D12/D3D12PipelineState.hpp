#pragma once

#include "D3D12RootSignature.hpp"
#include "D3D12Shader.hpp"
#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3dx12/d3dx12.h>

/*
	For now there are two pipelines available: vertex and mesh.
	Vertex pipeline will likely be removed at some later stage.
*/

namespace Luden
{
	class D3D12Device;

	class D3D12PipelineState
	{
	public:
		D3D12PipelineState() = default;
		~D3D12PipelineState() = default;

		Ref<ID3D12PipelineState>& GetHandle()
		{
			return m_PipelineState;
		}

		ID3D12PipelineState* GetHandleRaw()
		{
			return m_PipelineState;
		}

		PipelineType GetPipelineType() const
		{
			return m_PipelineType;
		}

		void SetName(std::string_view Name);

		Ref<ID3D12PipelineState> m_PipelineState;

	protected:

		PipelineType m_PipelineType;

	};

	class D3D12MeshPipelineState
	{

	};

	// Vertex Shading Pipeline
	class D3D12PipelineStateBuilder
	{
	public:
		D3D12PipelineStateBuilder(D3D12Device* pDevice);

		HRESULT Build(D3D12PipelineState& Pipeline);

		void SetRootSignature(D3D12RootSignature* pRootSignature);

		void SetVertexShader(D3D12Shader* pShader);
		void SetPixelShader(D3D12Shader* pShader);

		void EnableDepth(bool bEnable);

		void SetCullMode(D3D12_CULL_MODE CullMode);
		void SetFillMode(D3D12_FILL_MODE FillMode);

		void SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology);

		void SetDepthFormat(DXGI_FORMAT Format);

		void SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& Formats);

		void SetSampleCount(uint32 Sample, uint32 Quality = 1);

	private:
		D3D12Device* m_Device = nullptr;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc{};

		CD3DX12_RASTERIZER_DESC		m_RasterizerDesc{};
		CD3DX12_DEPTH_STENCIL_DESC	m_DepthDesc{};
		CD3DX12_BLEND_DESC			m_BlendDesc{};

		D3D12_CULL_MODE m_CullMode;
		D3D12_FILL_MODE m_FillMode;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_Topology;
		
		DXGI_FORMAT m_DepthFormat = DXGI_FORMAT_D32_FLOAT;

		std::vector<DXGI_FORMAT> m_RenderTargetFormats;

	};

	// Mesh Shading Pipeline
	class D3D12MeshPipelineStateBuilder
	{
	public:
		D3D12MeshPipelineStateBuilder(D3D12Device* pDevice);
	
		HRESULT Build(D3D12PipelineState& Pipeline);

		void SetAmplificationShader(D3D12Shader* pShader);
		void SetMeshShader(D3D12Shader* pShader);
		void SetPixelShader(D3D12Shader* pShader);
	
		void SetRootSignature(D3D12RootSignature* pRootSignature);

		void EnableDepth(bool bEnable);

		void SetCullMode(D3D12_CULL_MODE CullMode);
		void SetFillMode(D3D12_FILL_MODE FillMode);

		void SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology);

		void SetDepthFormat(DXGI_FORMAT Format);

		void SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& Formats);

	private:
		D3D12Device* m_Device = nullptr;

		D3DX12_MESH_SHADER_PIPELINE_STATE_DESC m_Desc{};
		 
		CD3DX12_RASTERIZER_DESC2		m_RasterizerDesc{};
		CD3DX12_DEPTH_STENCIL_DESC2		m_DepthDesc{};
		CD3DX12_BLEND_DESC				m_BlendDesc{};

		DXGI_FORMAT m_DepthFormat = DXGI_FORMAT_D32_FLOAT;

	};

} // namespace Luden
