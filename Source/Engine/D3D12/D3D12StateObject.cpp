#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12StateObject.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
	D3D12StateObject::D3D12StateObject()
		: RayGen(nullptr), Miss(nullptr), ClosestHit(nullptr)
	{
		
	}

	D3D12StateObject::~D3D12StateObject()
	{
		Release();
	}

	void D3D12StateObject::Release()
	{

	}

	D3D12StateObjectBuilder::D3D12StateObjectBuilder()
	{
		m_Desc = CD3DX12_STATE_OBJECT_DESC(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	}

	void D3D12StateObjectBuilder::Build(D3D12Device* pDevice, D3D12StateObject& Output)
	{
		auto shaderConfig = m_Desc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(m_PayloadSize, m_AttributeSize);

		auto pipelineConfig = m_Desc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(m_MaxRecursion);

		for (auto& record : m_HitGroups)
		{
			auto hitGroup = m_Desc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

			hitGroup->SetHitGroupType(record.Type);
			hitGroup->SetHitGroupExport(String::ToWide(record.Name).c_str());

			if (!record.ClosestHitName.empty())
			{
				hitGroup->SetClosestHitShaderImport(String::ToWide(record.ClosestHitName).c_str());
			}

			if (!record.AnyHitName.empty())
			{
				hitGroup->SetAnyHitShaderImport(String::ToWide(record.AnyHitName).c_str());
			}

			if (!record.IntersectionName.empty())
			{
				hitGroup->SetIntersectionShaderImport(String::ToWide(record.IntersectionName).c_str());
			}
		}	

		Output.RayGen = m_RayGenShader;
		Output.Miss = m_MissShader;
		Output.ClosestHit = m_ClosestHitShader;
		Output.PayloadSize = m_PayloadSize;

		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateStateObject(m_Desc, IID_PPV_ARGS(&Output.GetHandle())));
		VERIFY_D3D12_RESULT(Output.GetHandle()->QueryInterface(IID_PPV_ARGS(&Output.m_StateObjectProperties)));
	}

	void D3D12StateObjectBuilder::AddRayGen(D3D12Shader* pShader, std::vector<LPCWSTR> Exports)
	{
		const auto bytecode = CD3DX12_SHADER_BYTECODE(pShader->Data, pShader->Size);

		auto rayGenLib = m_Desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		rayGenLib->SetDXILLibrary(&bytecode);
		rayGenLib->DefineExports(Exports.data(), static_cast<uint32>(Exports.size()));

		m_RayGenShader = pShader;

	}

	void D3D12StateObjectBuilder::AddMiss(D3D12Shader* pShader, std::vector<LPCWSTR> Exports)
	{
		const auto bytecode = CD3DX12_SHADER_BYTECODE(pShader->Data, pShader->Size);

		auto missLib = m_Desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		missLib->SetDXILLibrary(&bytecode);
		missLib->DefineExports(Exports.data(), static_cast<uint32>(Exports.size()));

		m_MissShader = pShader;
	}

	void D3D12StateObjectBuilder::AddClosestHit(D3D12Shader* pShader, std::vector<LPCWSTR> Exports)
	{
		const auto bytecode = CD3DX12_SHADER_BYTECODE(pShader->Data, pShader->Size);

		auto closestHitLib = m_Desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		closestHitLib->SetDXILLibrary(&bytecode);
		closestHitLib->DefineExports(Exports.data(), static_cast<uint32>(Exports.size()));

		m_ClosestHitShader = pShader;

	}

	void D3D12StateObjectBuilder::AddHitGroup(FHitGroup HitGroup)
	{
		m_HitGroups.push_back(std::move(HitGroup));
	}

	void D3D12StateObjectBuilder::SetMaxRayRecursion(uint32 MaxRecursion)
	{
		m_MaxRecursion = MaxRecursion;
	}

	void D3D12StateObjectBuilder::SetPayloadSize(uint32 PayloadSize)
	{
		m_PayloadSize = PayloadSize;
	}

	void D3D12StateObjectBuilder::SetStateObjectType(D3D12_STATE_OBJECT_TYPE Type)
	{
		m_Desc.SetStateObjectType(Type);
	}

	void D3D12StateObjectBuilder::SetGlobalRootSignature(D3D12RootSignature* pRootSignature)
	{
		auto globalRootSignature = m_Desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(pRootSignature->GetHandleRaw());
	}

	void D3D12StateObjectBuilder::SetGlobalRootSignature(D3D12RootSignature* pRootSignature, std::vector<LPCWSTR> Exports)
	{
		auto globalRootSignature = m_Desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(pRootSignature->GetHandleRaw());

		auto associations = m_Desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		associations->SetSubobjectToAssociate(*globalRootSignature);
		associations->AddExports(Exports.data(), static_cast<uint32>(Exports.size()));
	}

} // namespace Luden
