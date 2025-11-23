#pragma once

#include <Core/Math/Math.hpp>
#include <Core/String.hpp>
#include "D3D12Shader.hpp"
#include "D3D12Resource.hpp"
#include <vector>

namespace Luden
{
	class D3D12StateObject;

	struct FShaderIdentifier
	{
		FShaderIdentifier() = default;
		FShaderIdentifier(void* pData)
		{
			Data = pData;
			//std::memcpy(Data, pData, ShaderIdentifierSizeInBytes);
		}

		void* Data;
		uint32 ShaderIdentifierSizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	};

	struct FShaderTableRecord
	{
		FShaderTableRecord() = default;
		FShaderTableRecord(const FShaderIdentifier& Identifier) 
			: Identifier(Identifier.Data), RootArgs(nullptr)
		{}
		FShaderTableRecord(FShaderIdentifier Identifier, void* pRootArgs, usize ArgsSize) 
			: Identifier(Identifier), RootArgs(pRootArgs), ArgsSize(static_cast<uint32>(ArgsSize))
		{
			TotalSize = Identifier.ShaderIdentifierSizeInBytes + static_cast<uint32>(ArgsSize);
		}

		FShaderIdentifier Identifier;
		void* RootArgs;
		uint32 ArgsSize = 0;

		uint32 TotalSize = 0;
	};

	// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#shader-identifier
	// Table per shader type.
	// TODO: requires rework.
	class D3D12ShaderTable
	{
	public:
		D3D12ShaderTable();
		~D3D12ShaderTable();

		void Create(D3D12Device* pDevice, std::string_view Name = "");

		void AddRecord(const FShaderTableRecord& Record)
		{
			m_Records.push_back(Record);
			
			m_Stride	= GetStrideInBytes();
			m_TotalSize = GetTotalSizeInBytes();
		}

		// Clamp to max 4096 bytes?
		uint64 GetStrideInBytes() const
		{
			return Math::Align<uint64>(sizeof(FShaderTableRecord), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		}
		
		uint64 GetTotalSizeInBytes()
		{
			uint64 size = static_cast<uint64>(m_Records.size() * GetStrideInBytes());
			return Math::Align<uint64>(size, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		}

		// Structured Buffer to hold and upload all records.
		uint32 StorageBuffer;
		
		uint64 GetTotalSize() const { return m_TotalSize; }

	private:
		uint64 m_TotalSize = 0;
		uint64 m_Stride = 0;
		
		std::vector<FShaderTableRecord> m_Records;

	};

	class D3D12ShaderBindingTable
	{
	public:
		D3D12ShaderBindingTable();
		~D3D12ShaderBindingTable();

		void Create(D3D12Device* pDevice, D3D12StateObject* pStateObject);

		D3D12Resource* GetStorageBuffer() { return m_StorageBuffer; }

		void AddRayGenRecord(const FShaderIdentifier& ShaderIdentifer);
		void AddMissRecord(const FShaderIdentifier& ShaderIdentifer);
		void AddHitGroupRecord(const FShaderIdentifier& ShaderIdentifer);

		D3D12ShaderTable RayGenTable;
		D3D12ShaderTable MissTable;
		D3D12ShaderTable HitTable;

		uint64 TotalSizeInBytes = 0;

		uint64 RayGenOffset = 0;
		uint64 MissOffset = 0;
		uint64 HitOffset = 0;

		void* m_CpuData;

	private:
		D3D12Resource* m_StorageBuffer;

		FShaderTableRecord				m_RayGenRecord;
		std::vector<FShaderTableRecord>	m_MissRecords;
		std::vector<FShaderTableRecord>	m_HitRecords;

		// https://github.com/ArtemVetik/Raytracing-DX12/blob/main/RaytracingDemo/RenderEngine/DXRHelpers/nv_helpers_dx12/ShaderBindingTableGenerator.cpp
		//void GetTotalSize();

		// For now should always be 64 bytes.
		//void GetMaxShaderEntrySize();

		// Copy each shader entry into SBT buffer.
		//void CopyShaderData();

		const uint64 m_ShaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	};

} // namespace Luden
