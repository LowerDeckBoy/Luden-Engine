#pragma once

#include "Graphics/Model.hpp"

namespace Luden
{
	class D3D12Device;
	class D3D12CommandList;
	class D3D12RootSignature;

	class D3D12AccelerationStructure
	{
	public:
		D3D12AccelerationStructure() = default;

		D3D12Buffer* AccelerationStructure = nullptr;
		D3D12Buffer* ScratchBuffer = nullptr;

		void Create();
		void Release();

		uint64 ScratchSize = 0;
		uint64 ResultSize  = 0;
		uint64 UpdateSize  = 0;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC	BuildDesc{};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS	BuildFlags{};

	};

	class D3D12BLAS : public D3D12AccelerationStructure
	{
	public:
		D3D12BLAS() = default;
		D3D12BLAS(D3D12Device* pDevice, D3D12CommandList* pCommandList, StaticMesh& Mesh);
		D3D12BLAS(D3D12RHI* pD3D12RHI, Model* pModel);
		~D3D12BLAS();

		void Create(D3D12Device* pDevice, D3D12CommandList* pCommandList);
		void Create(D3D12RHI* pD3D12RHI);

		void AddGeometryDesc(D3D12Device* pDevice, StaticMesh& Mesh);
		
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>			GeometryDescs;
		
	}; // class D3D12BLAS

	// https://github.com/ArtemVetik/Raytracing-DX12/blob/main/RaytracingDemo/RenderEngine/AccelerationStructure.cpp
	// https://github.com/PappaNiels/IntroDXR/blob/main/code/DXR/Renderer/Attributes/TLAS.cpp
	// https://scispace.com/pdf/introduction-to-directx-raytracing-2ip1x4g9qb.pdf
	class D3D12TLAS : public D3D12AccelerationStructure
	{
	public:
		D3D12TLAS() = default;
		D3D12TLAS(D3D12RHI* pD3D12RHI);
		~D3D12TLAS();

		void Create();

		void AddBLAS(Model* pModel);

		D3D12Descriptor ShaderResourceView;
		D3D12Descriptor UnorderedAccessView;

		D3D12Buffer* InstanceBuffer;

		std::vector<FRaytracingInstanceDesc> Instances;
		uint64 InstanceDescsSize = 0;

		std::vector<D3D12BLAS*> BLASes;

	private:
		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
		D3D12RHI* m_RHI = nullptr;

	}; // class D3D12TLAS

	// Temporal naming
	class D3D12BVH
	{
	public:
		D3D12BVH() = default;
		D3D12BVH(D3D12RHI* pD3D12RHI);
		~D3D12BVH();

		void Build(D3D12RHI* pD3D12RHI, Scene* pActiveScene);

		// Release all resources of this BVH.
		void Release();

		D3D12TLAS* TLAS;
		
	private:
		D3D12RHI* m_D3D12RHI = nullptr;

	}; // class D3D12BVH

} // namespace Luden
