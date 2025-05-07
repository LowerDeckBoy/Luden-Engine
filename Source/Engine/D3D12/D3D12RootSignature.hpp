#pragma once

#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <vector>

namespace Luden
{
	class D3D12Device;
	class D3D12Shader;

	enum class PipelineType
	{
		Graphics,
		Compute
	};

	class D3D12RootSignature
	{
	public:
		D3D12RootSignature() = default;
		~D3D12RootSignature();

		Ref<ID3D12RootSignature>& GetHandle()
		{
			return m_RootSignature;
		}

		ID3D12RootSignature* GetHandleRaw()
		{
			return m_RootSignature.Get();
		}

		HRESULT Build(D3D12Device* pDevice, PipelineType Type);

		// Use Shader Bytcode to create Root Signature.
		// Note:
		// Passed Shader MUST have specified Root Signature!
		HRESULT BuildFromShader(D3D12Device* pDevice, D3D12Shader* pShader, PipelineType Type);

		void AddConstants(uint32 Count, uint32 RegisterSlot, uint32 RegisterSpace = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		void AddCBV(uint32 RegisterSlot, uint32 RegisterSpace = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		void AddSRV(uint32 RegisterSlot, uint32 RegisterSpace = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

		void AddStaticSampler(uint32 RegisterSlot, uint32 RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparisonFunc);

		// For Raytracing Root Signature only.
		void SetGlobal(bool bSet);

		PipelineType GetPipelineType() const
		{
			return m_PipelineType;
		}

	private:
		Ref<ID3D12RootSignature> m_RootSignature;

		PipelineType m_PipelineType{};

		std::vector<D3D12_ROOT_PARAMETER1> m_Parameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC1> m_StaticSamplers;

		bool bIsGlobal = false;

		D3D12_ROOT_SIGNATURE_FLAGS m_RootFlags =
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	};

} // namespace Luden
