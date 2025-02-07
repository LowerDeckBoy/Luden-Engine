#include "D3D12Device.hpp"
#include "D3D12Shader.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
	D3D12RootSignature::~D3D12RootSignature()
	{
		SAFE_RELEASE(m_RootSignature);
	}

	HRESULT D3D12RootSignature::Build(D3D12Device* pDevice, PipelineType Type)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc{};
		desc.Version					= D3D_ROOT_SIGNATURE_VERSION_1_2;
		desc.Desc_1_2.pParameters		= m_Parameters.data();
		desc.Desc_1_2.NumParameters		= static_cast<uint32>(m_Parameters.size());
		desc.Desc_1_2.pStaticSamplers	= m_StaticSamplers.data();
		desc.Desc_1_2.NumStaticSamplers = static_cast<uint32>(m_StaticSamplers.size());

		Ref<ID3DBlob> signature;
		Ref<ID3DBlob> error;

		VERIFY_D3D12_RESULT(D3D12SerializeVersionedRootSignature(&desc, &signature, &error));
		
		m_PipelineType = Type;

		return pDevice->LogicalDevice->CreateRootSignature(pDevice->NodeMask, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	}

	HRESULT D3D12RootSignature::BuildFromShader(D3D12Device* pDevice, D3D12Shader* pShader, PipelineType Type)
	{
		const auto& bytecode = pShader->Bytecode();

		m_PipelineType = Type;

		return pDevice->LogicalDevice->CreateRootSignature(pDevice->NodeMask, bytecode.pShaderBytecode, bytecode.BytecodeLength, IID_PPV_ARGS(&m_RootSignature));;
	}

	void D3D12RootSignature::AddConstants(uint32 Count, uint32 RegisterSlot, uint32 RegisterSpace, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		parameter.Constants.Num32BitValues	= Count;
		parameter.Constants.ShaderRegister	= RegisterSlot;
		parameter.Constants.RegisterSpace	= RegisterSpace;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddCBV(uint32 RegisterSlot, uint32 RegisterSpace, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameter.Descriptor.ShaderRegister = RegisterSlot;
		parameter.Descriptor.RegisterSpace	= RegisterSpace;
		parameter.Descriptor.Flags			= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddStaticSampler(uint32 RegisterSlot, uint32 RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparisonFunc)
	{
		D3D12_STATIC_SAMPLER_DESC1 desc{};
		desc.ShaderRegister = RegisterSlot;
		desc.RegisterSpace	= RegisterSpace;
		desc.Filter			= Filter;
		desc.AddressU		= AddressMode;
		desc.AddressV		= AddressMode;
		desc.AddressW		= AddressMode;
		desc.ComparisonFunc = ComparisonFunc;
		desc.MaxAnisotropy	= D3D12_MAX_MAXANISOTROPY;
		desc.MinLOD			= 0.0f;
		desc.MaxLOD			= D3D12_FLOAT32_MAX;
		desc.BorderColor	= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		desc.Flags			= D3D12_SAMPLER_FLAG_NONE;
		
		m_StaticSamplers.emplace_back(desc);
	}

	static void SerializeRootSignature()
	{

	}
} // namespace Luden
