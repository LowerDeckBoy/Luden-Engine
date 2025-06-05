#pragma once

#include <Core/Math/Math.hpp>
#include <Core/String.hpp>
#include "D3D12Shader.hpp"
#include "D3D12Resource.hpp"
#include <vector>

namespace Luden
{
	struct FShaderIdentifier
	{
		FShaderIdentifier() = default;
		FShaderIdentifier(void* pData)
		{
			std::memcpy(Data, pData, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		}

		void* Data = nullptr;
		//uint32 Size = 0;
	};

	struct FShaderTableRecord
	{

		FShaderIdentifier Identifier;

		uint32 TotalSize = 0;

	};

	// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#shader-identifier
	class D3D12ShaderTable
	{
	public:
		D3D12ShaderTable(uint64 NumRecords)
			: m_Records(NumRecords) { }

		//void AddRecord(FShaderTableRecord Record);

		//void AddRayGenShader(std::string_view Name);
		//void AddMissShader(std::string_view Name, uint32 RayIndex);
		//void AddClosestHitShader(std::string_view Name, uint32 HitGroup);

		//uint64 GetTotalSizeInBytes();
		//uint64 GetSizeInBytes();

		// Clamp to max 4096 bytes?
		uint64 GetStride() const
		{
			return Math::Align<uint64>(sizeof(FShaderTableRecord), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		}


	private:
		uint64 m_TotalSize = 0;
		uint64 m_Stride = 0;
		
		std::vector<FShaderTableRecord> m_Records;

		std::vector<FShaderTableRecord> m_RayGenRecords;
		std::vector<FShaderTableRecord> m_MissRecords;
		std::vector<FShaderTableRecord> m_ClosestHitRecords;

		D3D12Resource* m_Storage;



	};
} // namespace Luden
