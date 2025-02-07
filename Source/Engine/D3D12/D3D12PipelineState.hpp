#pragma once

#include "D3D12RootSignature.hpp"
#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3dx12/d3dx12.h>
#include "D3D12Shader.hpp"

namespace Luden
{
	class D3D12Device;

	//enum class PipelineType
	//{
	//	Graphics,
	//	Compute
	//};

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

	private:
		Ref<ID3D12PipelineState> m_PipelineState;
		PipelineType m_PipelineType;

	};

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

		void SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		void SetDepthFormat(DXGI_FORMAT Format);

		void SetRenderTargetFormats(const std::vector<DXGI_FORMAT>& Formats);

		void SetSampleCount(uint32 Sample, uint32 Quality = 1);

	private:
		D3D12Device* m_Device = nullptr;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc{};

		CD3DX12_RASTERIZER_DESC2	m_RasterizerDesc{};
		CD3DX12_DEPTH_STENCIL_DESC2	m_DepthDesc{};
		CD3DX12_BLEND_DESC			m_BlendDesc{};

		D3D12_CULL_MODE m_CullMode;
		D3D12_FILL_MODE m_FillMode;

		D3D12_PRIMITIVE_TOPOLOGY m_Topology;
		
		DXGI_FORMAT m_DepthFormat = DXGI_FORMAT_D32_FLOAT;

		std::vector<DXGI_FORMAT> m_RenderTargetFormats;

	};

	class D3D12MeshPipelineState
	{
	public:
		D3D12MeshPipelineState(D3D12Device* pDevice);
	
		HRESULT Build(D3D12PipelineState& Pipeline);

		void SetAmplificationShader(D3D12Shader* pShader);
		void SetMeshShader(D3D12Shader* pShader);
		void SetPixelShader();
	
		void EnableDepth(bool bEnable);
		void SetWireframe(bool bEnable);

		void SetCullMode(D3D12_CULL_MODE CullMode);

		void SetPrimitiveTypeTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		void SetDepthFormat(DXGI_FORMAT Format);

	private:
		D3D12Device* m_Device = nullptr;

		D3DX12_MESH_SHADER_PIPELINE_STATE_DESC m_Desc{};

		CD3DX12_RASTERIZER_DESC2	m_RasterizerDesc{};
		CD3DX12_DEPTH_STENCIL_DESC2	m_DepthDesc{};
		CD3DX12_BLEND_DESC			m_BlendDesc{};

		D3D12_CULL_MODE m_CullMode;
		D3D12_FILL_MODE m_FillMode;

		D3D12_PRIMITIVE_TOPOLOGY m_Topology;

		DXGI_FORMAT m_DepthFormat = DXGI_FORMAT_D32_FLOAT;

	};

} // namespace Luden
