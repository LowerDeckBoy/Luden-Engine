#include "D3D12RHI.hpp"
#include "D3D12Raytracing.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Memory.hpp"
#include <Core/Math/Math.hpp>

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

	D3D12BLAS::D3D12BLAS(D3D12Device* pDevice, D3D12CommandList* pCommandList, StaticMesh& /* Mesh */)
	{
		Create(pDevice, pCommandList);
	}

	D3D12BLAS::~D3D12BLAS()
	{
		Release();
	}

	// https://github.com/zhaijialong/RealEngine/blob/main/source/gfx/d3d12/d3d12_rt_blas.cpp
	void D3D12BLAS::Create(D3D12Device* pDevice, D3D12CommandList* pCommandList)
	{
		Inputs.Type				= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		Inputs.Flags			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;// | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		Inputs.DescsLayout		= D3D12_ELEMENTS_LAYOUT_ARRAY;
		Inputs.NumDescs			= static_cast<uint32>(GeometryDescs.size());
		Inputs.pGeometryDescs	= GeometryDescs.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		pDevice->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&Inputs, &prebuildInfo);

		ScratchSize = ALIGN(prebuildInfo.ScratchDataSizeInBytes,		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		ResultSize  = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		UpdateSize  = ALIGN(prebuildInfo.UpdateScratchDataSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		ScratchBuffer = new D3D12Buffer(pDevice, BufferDesc{
			.BufferUsage = BufferUsageFlag::UnorderedAccess,
			.Size = ScratchSize,
			.bBindless = false,
			//.Name = "BLAS Scratch Buffer",
			});

		AccelerationStructure = new D3D12Buffer(pDevice, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = ResultSize,
			.bBindless = false,
			//.Name = "BLAS Result Buffer"
			});

		BuildDesc.Inputs = Inputs;
		BuildDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGpuAddress();
		BuildDesc.DestAccelerationStructureData = AccelerationStructure->GetGpuAddress();
		BuildDesc.SourceAccelerationStructureData = 0;

		pCommandList->BuildRaytracingAccelerationStructure(BuildDesc, 0, nullptr);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = AccelerationStructure->GetHandleRaw();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		pCommandList->GetHandle()->ResourceBarrier(1, &uavBarrier);


	}

	void D3D12BLAS::AddGeometryDesc(D3D12Device* pDevice, StaticMesh& Mesh)
	{
		auto vertexBuffer = pDevice->Buffers.at(Mesh.VertexBuffer);
		auto indexBuffer  = pDevice->Buffers.at(Mesh.IndexBuffer);

		D3D12_RAYTRACING_GEOMETRY_DESC desc{};

		desc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		desc.Triangles.VertexBuffer.StartAddress  = vertexBuffer->GetGpuAddress();
		desc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetBufferDesc().Stride;
		desc.Triangles.VertexCount  = vertexBuffer->GetBufferDesc().NumElements;
		desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

		desc.Triangles.IndexBuffer	= indexBuffer->GetGpuAddress();
		desc.Triangles.IndexCount	= indexBuffer->GetBufferDesc().NumElements;
		desc.Triangles.IndexFormat	= DXGI_FORMAT_R32_UINT;

		desc.Triangles.Transform3x4 = 0;

		GeometryDescs.push_back(std::move(desc));
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
		auto commandList = m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList;

		if (!commandList->IsOpen())
		{
			commandList->Open();
		}

		//for (auto& blas : BLASes)
		//{
		//	commandList->BuildRaytracingAccelerationStructure(blas->BuildDesc, 0, nullptr);
		//
		//	D3D12_RESOURCE_BARRIER uavBarrier{};
		//	uavBarrier.Type				= D3D12_RESOURCE_BARRIER_TYPE_UAV;
		//	uavBarrier.UAV.pResource	= blas->AccelerationStructure->GetHandleRaw();
		//	uavBarrier.Flags			= D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//	commandList->GetHandle()->ResourceBarrier(1, &uavBarrier);
		//}

		TLAS->Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		TLAS->Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		TLAS->Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		TLAS->Inputs.NumDescs = static_cast<uint32>(Instances.size());

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		m_D3D12RHI->Device->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&TLAS->Inputs, &prebuildInfo);

		TLAS->ScratchSize		= ALIGN(prebuildInfo.ScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS->ResultSize		= ALIGN(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS->InstanceDescsSize = ALIGN(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<uint64>(Instances.size()), D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT);

		auto heapProps = D3D::HeapPropertiesDefault();
		
		TLAS->ScratchBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::UnorderedAccess,
			.Size = TLAS->ScratchSize,
			.bBindless = false,
			.Name = "D3D12 TLAS Scratch Buffer",
			});
		
		TLAS->AccelerationStructure = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
			.BufferUsage = BufferUsageFlag::AccelerationStructure,
			.Size = TLAS->ResultSize,
			.bBindless = false,
			.Name = "D3D12 TLAS Result Buffer"
			});

		//TLAS->InstanceBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
		//	.BufferUsage = BufferUsageFlag::Structured,
		//	.Size = TLAS->InstanceDescsSize,
		//	.bBindless = false,
		//	.Name = "D3D12 TLAS Instance Descs Buffer"
		//	});

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
		
		std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs(Instances.size());
		for (uint32 instanceIdx = 0; instanceIdx < Instances.size(); ++instanceIdx)
		{
			instanceDescs[instanceIdx].AccelerationStructure = Instances.at(instanceIdx).AccelerationStructure;
			instanceDescs[instanceIdx].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
			instanceDescs[instanceIdx].InstanceMask = 0xFF;
			instanceDescs[instanceIdx].InstanceID = Instances.at(instanceIdx).InstanceID;
			instanceDescs[instanceIdx].InstanceContributionToHitGroupIndex = instanceIdx;
			//instanceDescs[instanceIdx].InstanceContributionToHitGroupIndex = Instances.at(instanceIdx).InstanceContributionToHitGroupIndex;

			DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat3x4(&Instances.at(instanceIdx).Transform));
			DirectX::XMFLOAT3X4 transform{};
			DirectX::XMStoreFloat3x4(&transform, matrix);
			std::memcpy(instanceDescs[instanceIdx].Transform, &transform, sizeof(instanceDescs[instanceIdx].Transform));
		}
		
		//TLAS->InstanceBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
		//	.BufferUsage = BufferUsageFlag::Structured,
		//	.Data = instanceDescs.data(),
		//	.NumElements = static_cast<uint32>(Instances.size()),
		//	.Stride = sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		//	.Size = static_cast<uint32>(Instances.size()) * sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		//	.bBindless = false
		//	});

		TLAS->InstanceBuffer->GetHandle()->Map(0, nullptr, &pData);
		std::memcpy(pData, instanceDescs.data(), instanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		TLAS->InstanceBuffer->GetHandle()->Unmap(0, nullptr);

		//D3D12UploadContext::UploadBuffer(TLAS->AccelerationStructure, TLAS->ResultSize);
		//D3D12UploadContext::UploadBuffer(TLAS->InstanceBuffer, static_cast<uint32>(Instances.size()) * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		//D3D12UploadContext::Upload();

		TLAS->Inputs.InstanceDescs	= TLAS->InstanceBuffer->GetGpuAddress();

		TLAS->BuildDesc.Inputs = TLAS->Inputs;
		TLAS->BuildDesc.ScratchAccelerationStructureData = TLAS->ScratchBuffer->GetGpuAddress();
		TLAS->BuildDesc.DestAccelerationStructureData = TLAS->AccelerationStructure->GetGpuAddress();

		commandList->BuildRaytracingAccelerationStructure(TLAS->BuildDesc, 0, nullptr);

		m_D3D12RHI->Device->ShaderResourceHeap->Allocate(TLAS->AccelerationStructure->ShaderResourceView);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = TLAS->AccelerationStructure->GetGpuAddress();

		m_D3D12RHI->Device->LogicalDevice->CreateShaderResourceView(nullptr, &srvDesc, TLAS->AccelerationStructure->ShaderResourceView.CpuHandle);
		
		m_D3D12RHI->GraphicsQueue->Execute({ commandList });

	}

	void D3D12BVH::AddBLAS(Model* pModel, D3D12CommandList* pCommandList)
	{
		D3D12BLAS* blas = new D3D12BLAS();

		for (auto& mesh : pModel->Meshes)
		{
			blas->AddGeometryDesc(m_D3D12RHI->Device, mesh);
		}

		blas->Create(m_D3D12RHI->Device, pCommandList);

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
