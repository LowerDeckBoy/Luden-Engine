#include "D3D12Device.hpp"
#include "D3D12ShaderTable.hpp"
#include "D3D12Utility.hpp"
#include "D3D12UploadContext.hpp"

namespace Luden
{
	D3D12ShaderBindingTable::D3D12ShaderBindingTable()
	{
		m_StorageBuffer = new D3D12Resource();
		
		//m_StorageUploadBuffer = new D3D12Resource();
	}

	D3D12ShaderBindingTable::~D3D12ShaderBindingTable()
	{
		m_StorageBuffer->Release();
	}

	void D3D12ShaderBindingTable::Create(D3D12Device* pDevice)
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

		RayGenTable.Create(pDevice);
		MissTable.Create(pDevice);
		HitTable.Create(pDevice);

		const auto& raygenBuffer = pDevice->Buffers.at(RayGenTable.StorageBuffer);
		const auto& missBuffer = pDevice->Buffers.at(MissTable.StorageBuffer);
		const auto& hitBuffer = pDevice->Buffers.at(HitTable.StorageBuffer);
		D3D12UploadContext::UploadBuffer(raygenBuffer, RayGenTable.GetTotalSize());
		D3D12UploadContext::UploadBuffer(missBuffer, MissTable.GetTotalSize());
		D3D12UploadContext::UploadBuffer(hitBuffer, HitTable.GetTotalSize());
		D3D12UploadContext::Upload();

		void* mapped;
		VERIFY_D3D12_RESULT(m_StorageBuffer->GetHandle()->Map(0, nullptr, &mapped));
		//std::memcpy(mapped, &m_CpuData, TotalSizeInBytes);
		//m_StorageBuffer->GetHandle()->Unmap(0, nullptr);

	}

	D3D12ShaderTable::D3D12ShaderTable()
	{
		
	}

	D3D12ShaderTable::~D3D12ShaderTable()
	{
		
	}

	void D3D12ShaderTable::Create(D3D12Device* pDevice)
	{
		BufferDesc desc{};
		desc.Data			= m_Records.data();
		desc.Stride			= static_cast<uint32>(GetStrideInBytes());
		desc.NumElements	= static_cast<uint32>(m_Records.size());
		desc.BufferUsage	= BufferUsageFlag::Structured;
		desc.bBindless		= true;
		desc.Size			= GetTotalSize();
		StorageBuffer = pDevice->CreateBuffer(desc);

	}
} // namespace Luden
