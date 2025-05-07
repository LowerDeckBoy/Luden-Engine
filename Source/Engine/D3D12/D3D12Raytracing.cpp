#include "D3D12RHI.hpp"
#include "D3D12Raytracing.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Memory.hpp"
#include <Core/Math/Math.hpp>
//#include "Graphics/Model.hpp"


namespace Luden
{
	void D3D12AccelerationStructure::Release()
	{
		if (AccelerationStructure)
		{
			AccelerationStructure->Release();
		}

		if (ScratchBuffer)
		{
			ScratchBuffer->Release();
		}
	}

	D3D12BLAS::D3D12BLAS(D3D12Device* pDevice, StaticMesh& Mesh)
	{
		Create(pDevice);
	}

	D3D12BLAS::~D3D12BLAS()
	{
		Release();
	}

	void D3D12BLAS::Create(D3D12Device* pDevice)
	{
		/*
		auto vertexBuffer  = pDevice->Buffers.at(Mesh.VertexBuffer);
		auto indexBuffer   = pDevice->Buffers.at(Mesh.IndexBuffer);

		GeometryDesc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		GeometryDesc.Triangles.VertexBuffer.StartAddress  = vertexBuffer->GetGpuAddress();
		GeometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetBufferDesc().Stride;
		GeometryDesc.Triangles.VertexCount  = vertexBuffer->GetBufferDesc().NumElements;
		GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

		GeometryDesc.Triangles.IndexBuffer  = indexBuffer->GetGpuAddress();
		GeometryDesc.Triangles.IndexCount   = indexBuffer->GetBufferDesc().NumElements;
		GeometryDesc.Triangles.IndexFormat  = DXGI_FORMAT_R32_UINT;

		GeometryDesc.Triangles.Transform3x4 = 0;
		*/

		Inputs.Type  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		Inputs.pGeometryDescs = GeometryDescs.data();
		Inputs.NumDescs = static_cast<uint32>(GeometryDescs.size());

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		pDevice->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&Inputs, &prebuildInfo);

		ScratchSize = ALIGN(prebuildInfo.ScratchDataSizeInBytes,		 D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		ResultSize  = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,	 D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		UpdateSize  = ALIGN(prebuildInfo.UpdateScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		ScratchBuffer = new D3D12Buffer(pDevice, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = ScratchSize,
			.bBindless = false,
			.Name = "BLAS Scratch Buffer",
			});

		AccelerationStructure = new D3D12Buffer(pDevice, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = ResultSize,
			.bBindless = false,
			.Name = "BLAS Result Buffer"
			});

		
		BuildDesc.Inputs = Inputs;
		BuildDesc.ScratchAccelerationStructureData	= ScratchBuffer->GetGpuAddress();
		BuildDesc.DestAccelerationStructureData		= AccelerationStructure->GetGpuAddress();

	}

	void D3D12BLAS::AddGeometryDesc(D3D12Device* pDevice, StaticMesh& Mesh)
	{
		auto vertexBuffer = pDevice->Buffers.at(Mesh.VertexBuffer);
		auto indexBuffer = pDevice->Buffers.at(Mesh.IndexBuffer);

		D3D12_RAYTRACING_GEOMETRY_DESC desc{};

		desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		desc.Triangles.VertexBuffer.StartAddress  = vertexBuffer->GetGpuAddress();
		desc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetBufferDesc().Stride;
		desc.Triangles.VertexCount  = vertexBuffer->GetBufferDesc().NumElements;
		desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

		desc.Triangles.IndexBuffer	= indexBuffer->GetGpuAddress();
		desc.Triangles.IndexCount	= indexBuffer->GetBufferDesc().NumElements;
		desc.Triangles.IndexFormat	= DXGI_FORMAT_R32_UINT;

		desc.Triangles.Transform3x4 = 0;

		GeometryDescs.push_back(desc);
	}

	D3D12TLAS::D3D12TLAS(D3D12Device* pDevice)
	{
		m_ParentDevice = pDevice;
	}

	D3D12TLAS::~D3D12TLAS()
	{
		if (InstanceBuffer)
		{
			InstanceBuffer->Release();
		}

		Release();
	}

	D3D12BVH::D3D12BVH(D3D12RHI* pD3D12RHI)
		: m_D3D12RHI(pD3D12RHI)
	{
		TLAS = new D3D12TLAS(pD3D12RHI->Device);
	}

	D3D12BVH::~D3D12BVH()
	{
		Release();
	}

	void D3D12BVH::CreateTLAS()
	{
		TLAS->Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		TLAS->Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		TLAS->Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		TLAS->Inputs.NumDescs = static_cast<uint32>(Instances.size());
		//TLAS->Inputs.InstanceDescs = Instances.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		m_D3D12RHI->Device->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&TLAS->Inputs, &prebuildInfo);

		TLAS->ScratchSize		= ALIGN(prebuildInfo.ScratchDataSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS->ResultSize		= ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS->InstanceDescsSize = ALIGN(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<uint64>(Instances.size()), D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		
		/*
		TLAS->ScratchBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = TLAS->ScratchSize,
			.bBindless = false,
			.Name = "TLAS Scratch Buffer",
			});

		TLAS->AccelerationStructure = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = TLAS->ResultSize,
			.bBindless = false,
			.Name = "TLAS Result Buffer"
			});
		*/

		auto heapProps = D3D::HeapPropertiesDefault();
		
		TLAS->ScratchBuffer = new D3D12Buffer();
		auto scratchDesc = D3D12Buffer::CreateBufferDesc(TLAS->ScratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&scratchDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr, nullptr,
			IID_PPV_ARGS(&TLAS->ScratchBuffer->GetHandle())));
		
		TLAS->AccelerationStructure = new D3D12Buffer();
		auto resultDesc = D3D12Buffer::CreateBufferDesc(TLAS->ResultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resultDesc,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr, nullptr,
			IID_PPV_ARGS(&TLAS->AccelerationStructure->GetHandle())));
		
		TLAS->InstanceBuffer = new D3D12Buffer();
		auto instancesDesc = D3D12Buffer::CreateBufferDesc(TLAS->InstanceDescsSize, D3D12_RESOURCE_FLAG_NONE);
		heapProps = D3D::HeapPropertiesUpload();
		VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource2(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&instancesDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr, nullptr,
			IID_PPV_ARGS(&TLAS->InstanceBuffer->GetHandle())));

		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs{};
		VERIFY_D3D12_RESULT(TLAS->InstanceBuffer->GetHandle()->Map(0, nullptr, reinterpret_cast<void**>(&instanceDescs)));

		for (uint32 instanceIdx = 0; instanceIdx < Instances.size(); ++instanceIdx)
		{
			//instanceDescs[instanceIdx].AccelerationStructure = Instances.at(instanceIdx).BottomLevel->GetGPUVirtualAddress();
			instanceDescs[instanceIdx].AccelerationStructure = Instances.at(instanceIdx).AccelerationStructure;
			instanceDescs[instanceIdx].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
			instanceDescs[instanceIdx].InstanceMask = 0xFF;
			instanceDescs[instanceIdx].InstanceID = Instances.at(instanceIdx).InstanceID;
			instanceDescs[instanceIdx].InstanceContributionToHitGroupIndex = Instances.at(instanceIdx).InstanceContributionToHitGroupIndex;

			DirectX::XMMATRIX matrix = DirectX::XMLoadFloat3x4(&Instances.at(instanceIdx).Transform);
			DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(matrix);
			std::memcpy(instanceDescs[instanceIdx].Transform, &transform, sizeof(instanceDescs[instanceIdx].Transform));
		}
		TLAS->InstanceBuffer->GetHandle()->Unmap(0, nullptr);

		TLAS->Inputs.InstanceDescs = TLAS->InstanceBuffer->GetGpuAddress();

		TLAS->BuildDesc.Inputs = TLAS->Inputs;
		TLAS->BuildDesc.ScratchAccelerationStructureData = TLAS->ScratchBuffer->GetGpuAddress();
		TLAS->BuildDesc.DestAccelerationStructureData = TLAS->AccelerationStructure->GetGpuAddress();

		if (!m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList->IsOpen())
		{
			m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList->Open();
		}

		m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList->GetHandle()->BuildRaytracingAccelerationStructure(&TLAS->BuildDesc, 0, nullptr);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = TLAS->AccelerationStructure->GetHandleRaw();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList->GetHandle()->ResourceBarrier(1, &uavBarrier);

		m_D3D12RHI->GraphicsQueue->Execute({ m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList });


	}

	void D3D12BVH::AddBLAS(Model* pModel)
	{
		D3D12BLAS* blas = new D3D12BLAS();

		for (auto& mesh : pModel->Meshes)
		{
			blas->AddGeometryDesc(m_D3D12RHI->Device, mesh);

		}

		blas->Create(m_D3D12RHI->Device);

		for (auto& mesh : pModel->Meshes)
		{
			mesh.RaytracingInstanceDesc.AccelerationStructure = blas->AccelerationStructure->GetGpuAddress();
			Instances.push_back(mesh.RaytracingInstanceDesc);
		}

		BLASes.push_back(blas);
	}

	void D3D12BVH::Release()
	{
		for (auto blas : BLASes)
		{
			delete blas;
		}

		delete TLAS;
	}

} // namespace Luden
