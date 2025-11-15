#include "D3D12Device.hpp"
#include "D3D12StateObject.hpp"
#include "D3D12ShaderTable.hpp"
#include "D3D12Utility.hpp"
#include "D3D12UploadContext.hpp"

namespace Luden
{
	D3D12ShaderBindingTable::D3D12ShaderBindingTable()
	{
		m_StorageBuffer = new D3D12Resource();
	}

	D3D12ShaderBindingTable::~D3D12ShaderBindingTable()
	{
		m_StorageBuffer->Release();
	}

	void D3D12ShaderBindingTable::Create(D3D12Device* pDevice, D3D12StateObject* pStateObject)
	{
		RayGenOffset = static_cast<uint32>(TotalSizeInBytes);
		TotalSizeInBytes += RayGenTable.GetTotalSize();
		TotalSizeInBytes = Math::Align<uint64>(TotalSizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		MissOffset = static_cast<uint32>(TotalSizeInBytes);
		TotalSizeInBytes += MissTable.GetTotalSize();
		TotalSizeInBytes = Math::Align<uint64>(TotalSizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		HitOffset = static_cast<uint32>(TotalSizeInBytes);
		TotalSizeInBytes += HitTable.GetTotalSize();
		TotalSizeInBytes = Math::Align<uint64>(TotalSizeInBytes, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalSizeInBytes);
		const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_StorageBuffer->GetHandle())));

		void* rayGenIdentifier		= pStateObject->GetShaderIdentifier(L"RayGen");
		void* missIdentifier		= pStateObject->GetShaderIdentifier(L"Miss");
		void* closestHitIdentifier	= pStateObject->GetShaderIdentifier(L"HitGroup");

		uint8* start = (uint8*)m_CpuData;
		VERIFY_D3D12_RESULT(m_StorageBuffer->GetHandle()->Map(0, nullptr, reinterpret_cast<void**>(&start)));
		
		std::memcpy(start + RayGenOffset,	rayGenIdentifier,		m_ShaderIdentifierSize);
		std::memcpy(start + MissOffset,		missIdentifier,			m_ShaderIdentifierSize);
		std::memcpy(start + HitOffset,		closestHitIdentifier,	m_ShaderIdentifierSize);

		m_StorageBuffer->GetHandle()->Unmap(0, nullptr);

	}

	void D3D12ShaderBindingTable::AddRayGenRecord(const FShaderIdentifier& ShaderIdentifer)
	{
		m_RayGenRecord = FShaderTableRecord(ShaderIdentifer);
	}

	void D3D12ShaderBindingTable::AddMissRecord(const FShaderIdentifier& ShaderIdentifer)
	{
		m_MissRecords.push_back(FShaderTableRecord(ShaderIdentifer));
	}

	void D3D12ShaderBindingTable::AddHitGroupRecord(const FShaderIdentifier& ShaderIdentifer)
	{
		m_HitRecords.push_back(FShaderTableRecord(ShaderIdentifer));
	}

	D3D12ShaderTable::D3D12ShaderTable()
	{
		
	}

	D3D12ShaderTable::~D3D12ShaderTable()
	{
		
	}

	void D3D12ShaderTable::Create(D3D12Device* pDevice, std::string_view Name)
	{
		BufferDesc desc{};
		desc.Data			= m_Records.data();
		desc.Stride			= static_cast<uint32>(GetStrideInBytes());
		desc.NumElements	= static_cast<uint32>(m_Records.size());
		desc.BufferUsage	= BufferUsageFlag::Structured;
		desc.bBindless		= true;
		desc.Size			= GetTotalSize();
		desc.Size			= GetTotalSizeInBytes();
		
		if (!Name.empty())
		{
			desc.Name = Name.data();
		}

		StorageBuffer = pDevice->CreateBuffer(desc);

	}
} // namespace Luden
