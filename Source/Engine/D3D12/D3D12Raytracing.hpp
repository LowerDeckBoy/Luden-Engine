#pragma once

#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"
//#include "D3D12StateObject.hpp"
#include "D3D12RootSignature.hpp"
#include "Graphics/Model.hpp"

namespace Luden
{
	struct StaticMesh;
	class Model;

	//
	//https://renderingpixels.com/2022/07/getting-started-with-directx-raytracing/#bottom-level-acceleration-structure
	//

	class D3D12AccelerationStructure
	{
	public:

		D3D12Buffer* AccelerationStructure;
		D3D12Buffer* ScratchBuffer;

		void Create();

		void Release();

		uint64 ScratchSize = 0;
		uint64 ResultSize  = 0;
		uint64 UpdateSize  = 0;

	};

	class D3D12BLAS : public D3D12AccelerationStructure
	{
	public:
		D3D12BLAS() = default;
		D3D12BLAS(D3D12Device* pDevice, StaticMesh& Mesh);
		~D3D12BLAS();

		void Create(D3D12Device* pDevice);

		void AddGeometryDesc(D3D12Device* pDevice, StaticMesh& Mesh);

		D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc{};
		// TODO:
		// Put model's meshes into single BLAS
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>				GeometryDescs;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS	Inputs{};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC		BuildDesc{};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS		Flags{};
		
		
	}; // class D3D12BLAS

	class D3D12TLAS : public D3D12AccelerationStructure
	{
	public:
		D3D12TLAS() = default;
		D3D12TLAS(D3D12Device* pDevice);
		~D3D12TLAS();

		D3D12Descriptor ShaderResourceView;
		D3D12Descriptor UnorderedAccessView;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS	Inputs{};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC		BuildDesc{};

		D3D12Buffer* InstanceBuffer;

		uint64 InstanceDescsSize = 0;

	private:
		D3D12Device* m_ParentDevice = nullptr;


	}; // class D3D12TLAS

	// Temporal naming
	class D3D12BVH
	{
	public:
		D3D12BVH() = default;
		D3D12BVH(D3D12RHI* pD3D12RHI);
		~D3D12BVH();

		void CreateTLAS();

		void AddBLAS(Model* pModel);

		// Release all resources in this BVH.
		void Release();

		D3D12TLAS* TLAS;
		
		std::vector<D3D12BLAS*> BLASes;
		std::vector<FRaytracingInstanceDesc> Instances;


	private:
		D3D12RHI* m_D3D12RHI = nullptr;

	}; // class D3D12BVH

	struct FRaytracingPipelineSpecs
	{
		// How much data will a single ray carry.
		uint32 PayloadSize;
		uint32 AttributeSize = 8;
		// 2 -> Shadows
		uint32 MaxRecursion = 1;
	};

	class D3D12RaytracingPipeline
	{
	public:
		D3D12RaytracingPipeline(FRaytracingPipelineSpecs PipelineSpecs);

	private:
		//D3D12StateObject* m_StateObject;
		D3D12RootSignature* m_RootSignature;

		FRaytracingPipelineSpecs m_PipelineSpecs{};
	};
	
} // namespace Luden
