#include "Scene/Scene.hpp"
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

	D3D12BLAS::D3D12BLAS(D3D12RHI* pD3D12RHI, Model* pModel)
	{
		for (auto& mesh : pModel->OpaqueMeshes)
		{
			AddGeometryDesc(pD3D12RHI->Device, mesh);
		}

		Create(pD3D12RHI->Device, pD3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList);

	}

	D3D12BLAS::~D3D12BLAS()
	{
		Release();
	}

	// https://github.com/zhaijialong/RealEngine/blob/main/source/gfx/d3d12/d3d12_rt_blas.cpp
	// https://github.com/shikihuiku/D3D12MiniPathtracer/blob/main/src/Raytracing.cpp
	void D3D12BLAS::Create(D3D12Device* pDevice, D3D12CommandList* pCommandList)
	{
		BuildDesc.Inputs.Type			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		BuildDesc.Inputs.Flags			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;// | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		BuildDesc.Inputs.DescsLayout	= D3D12_ELEMENTS_LAYOUT_ARRAY;
		BuildDesc.Inputs.NumDescs		= static_cast<uint32>(GeometryDescs.size());
		BuildDesc.Inputs.pGeometryDescs	= GeometryDescs.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		pDevice->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&BuildDesc.Inputs, &prebuildInfo);

		ScratchSize = ALIGN(prebuildInfo.ScratchDataSizeInBytes,		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		ResultSize  = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		UpdateSize  = ALIGN(prebuildInfo.UpdateScratchDataSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		ScratchBuffer = new D3D12Buffer(pDevice, BufferDesc{
			.BufferUsage = BufferUsageFlag::UnorderedAccess,
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

		BuildDesc.Inputs							= BuildDesc.Inputs;
		BuildDesc.ScratchAccelerationStructureData	= ScratchBuffer->GetGpuAddress();
		BuildDesc.DestAccelerationStructureData		= AccelerationStructure->GetGpuAddress();

		pCommandList->BuildRaytracingAccelerationStructure(BuildDesc);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = AccelerationStructure->GetHandleRaw();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		pCommandList->GetHandle()->ResourceBarrier(1, &uavBarrier);
		
	}

	void D3D12BLAS::AddGeometryDesc(D3D12Device* pDevice, StaticMesh& Mesh)
	{
		auto vertexBuffer = pDevice->GetBuffer(Mesh.VertexBuffer);
		auto indexBuffer  = pDevice->GetBuffer(Mesh.IndexBuffer);

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

		GeometryDescs.push_back(desc);
	}

	D3D12TLAS::D3D12TLAS(D3D12RHI* pD3D12RHI)
	{
		InstanceBuffer = nullptr;
		m_D3D12RHI = pD3D12RHI;
	}

	D3D12TLAS::~D3D12TLAS()
	{
		if (InstanceBuffer)
		{
			InstanceBuffer->Release();
		}

		Release();
	}

	void D3D12TLAS::Create()
	{
		auto* commandList = m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList;
		if (!commandList->IsOpen())
		{
			commandList->Open();
		}

		// Get buffer sizes
		{
			BuildDesc.Inputs.Type			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			BuildDesc.Inputs.Flags			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
			BuildDesc.Inputs.DescsLayout	= D3D12_ELEMENTS_LAYOUT_ARRAY;
			BuildDesc.Inputs.NumDescs		= static_cast<uint32>(Instances.size());

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
			m_D3D12RHI->Device->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&BuildDesc.Inputs, &prebuildInfo);

			ScratchSize			= ALIGN(prebuildInfo.ScratchDataSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
			ResultSize			= ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
			InstanceDescsSize	= ALIGN(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<uint64>(Instances.size()), D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		}

		// Create buffers
		{
			InstanceBuffer = new D3D12Buffer();
			auto instancesDesc = D3D12Buffer::CreateBufferDesc(InstanceDescsSize, D3D12_RESOURCE_FLAG_NONE);
			auto heapProps = D3D::HeapPropertiesUpload();
			VERIFY_D3D12_RESULT(m_D3D12RHI->Device->LogicalDevice->CreateCommittedResource2(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&instancesDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr, nullptr,
				IID_PPV_ARGS(&InstanceBuffer->GetHandle())));

			VERIFY_D3D12_RESULT(InstanceBuffer->GetHandle()->Map(0, nullptr, reinterpret_cast<void**>(&m_InstanceDescs)));
			for (uint32 instanceIdx = 0; instanceIdx < Instances.size(); ++instanceIdx)
			{
				m_InstanceDescs[instanceIdx].AccelerationStructure = Instances.at(instanceIdx).AccelerationStructure;
				m_InstanceDescs[instanceIdx].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
				m_InstanceDescs[instanceIdx].InstanceMask = 0xFF;
				m_InstanceDescs[instanceIdx].InstanceID = instanceIdx;
				m_InstanceDescs[instanceIdx].InstanceContributionToHitGroupIndex = 0;

				DirectX::XMMATRIX matrix = DirectX::XMLoadFloat3x4(&Instances.at(instanceIdx).Transform);
				DirectX::XMFLOAT3X4 transform{};
				DirectX::XMStoreFloat3x4(&transform, matrix);
				std::memcpy(m_InstanceDescs[instanceIdx].Transform, &transform, sizeof(m_InstanceDescs[instanceIdx].Transform));
			}
			InstanceBuffer->GetHandle()->Unmap(0, nullptr);

			ScratchBuffer = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
				.BufferUsage = BufferUsageFlag::UnorderedAccess,
				.Size = ScratchSize,
				.bBindless = false,
				.Name = "D3D12 TLAS Scratch Buffer",
				});

			AccelerationStructure = new D3D12Buffer(m_D3D12RHI->Device, BufferDesc{
				.BufferUsage = BufferUsageFlag::AccelerationStructure,
				.Size = ResultSize,
				.bBindless = false,
				.Name = "D3D12 TLAS Result Buffer"
				});
		}
			
		BuildDesc.Inputs.Type	= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		BuildDesc.Inputs.Flags	= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		BuildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		BuildDesc.Inputs.pGeometryDescs = nullptr;
		BuildDesc.Inputs.NumDescs = static_cast<uint32>(Instances.size());
		BuildDesc.Inputs.InstanceDescs = InstanceBuffer->GetHandle()->GetGPUVirtualAddress();

		BuildDesc.SourceAccelerationStructureData	= 0;
		BuildDesc.ScratchAccelerationStructureData	= ScratchBuffer->GetGpuAddress();
		BuildDesc.DestAccelerationStructureData		= AccelerationStructure->GetGpuAddress();

		commandList->BuildRaytracingAccelerationStructure(BuildDesc);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = AccelerationStructure->GetHandleRaw();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		commandList->GetHandle()->ResourceBarrier(1, &uavBarrier);

	}

	void D3D12TLAS::AddBLAS(Model* pModel)
	{
		auto* device = m_D3D12RHI->Device;

		D3D12BLAS* blas = new D3D12BLAS();

		for (auto& mesh : pModel->OpaqueMeshes)
		{
			auto vertexBuffer = device->GetBuffer(mesh.VertexBuffer);
			auto indexBuffer  = device->GetBuffer(mesh.IndexBuffer);

			D3D12_RAYTRACING_GEOMETRY_DESC desc{};

			desc.Type	= D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			desc.Flags	= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			desc.Triangles.VertexBuffer.StartAddress	= vertexBuffer->GetGpuAddress();
			desc.Triangles.VertexBuffer.StrideInBytes	= vertexBuffer->GetBufferDesc().Stride;
			desc.Triangles.VertexCount	= vertexBuffer->GetBufferDesc().NumElements;
			desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

			desc.Triangles.IndexBuffer	= indexBuffer->GetGpuAddress();
			desc.Triangles.IndexCount	= indexBuffer->GetBufferDesc().NumElements;
			desc.Triangles.IndexFormat	= DXGI_FORMAT_R32_UINT;

			desc.Triangles.Transform3x4 = 0;

			blas->GeometryDescs.push_back(std::move(desc));
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.Type				= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags			= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		inputs.DescsLayout		= D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.NumDescs			= static_cast<uint32>(blas->GeometryDescs.size());
		inputs.pGeometryDescs	= blas->GeometryDescs.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		device->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

		blas->ScratchSize	= ALIGN(prebuildInfo.ScratchDataSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		blas->ResultSize	= ALIGN(prebuildInfo.ResultDataMaxSizeInBytes,	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		blas->ScratchBuffer = new D3D12Buffer(device, BufferDesc{
			.BufferUsage	= BufferUsageFlag::UnorderedAccess,
			.Size			= blas->ScratchSize,
			.bBindless		= false,
			});

		blas->AccelerationStructure = new D3D12Buffer(device, BufferDesc{
			.BufferUsage	= BufferUsageFlag::AccelerationStructure,
			.Size			= blas->ResultSize,
			.bBindless		= false,
			});

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.Inputs = inputs;
		buildDesc.SourceAccelerationStructureData	= 0;
		buildDesc.ScratchAccelerationStructureData	= blas->ScratchBuffer->GetGpuAddress();
		buildDesc.DestAccelerationStructureData		= blas->AccelerationStructure->GetGpuAddress();

		auto* commandList = m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList;

		if (!commandList->IsOpen())
		{
			commandList->Open();
		}

		commandList->BuildRaytracingAccelerationStructure(buildDesc);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = blas->AccelerationStructure->GetHandleRaw();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		commandList->GetHandle()->ResourceBarrier(1, &uavBarrier);

		for (auto& instance : pModel->Meshes)
		{
			instance.RaytracingInstanceDesc.AccelerationStructure = blas->AccelerationStructure->GetGpuAddress();
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			DirectX::XMStoreFloat3x4(&instance.RaytracingInstanceDesc.Transform, matrix);
			Instances.push_back(instance.RaytracingInstanceDesc);
		}

		blas->BuildDesc = buildDesc;

		BLASes.push_back(blas);

	}

	void D3D12TLAS::AddBLASPerMesh(Model* pModel)
	{
		auto* device = m_D3D12RHI->Device;

		for (auto& mesh : pModel->Meshes)
		{
			D3D12BLAS* blas = new D3D12BLAS();

			auto vertexBuffer = device->GetBuffer(mesh.VertexBuffer);
			auto indexBuffer = device->GetBuffer(mesh.IndexBuffer);

			D3D12_RAYTRACING_GEOMETRY_DESC desc{};

			desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			desc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGpuAddress();
			desc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetBufferDesc().Stride;
			desc.Triangles.VertexCount = vertexBuffer->GetBufferDesc().NumElements;
			desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

			desc.Triangles.IndexBuffer = indexBuffer->GetGpuAddress();
			desc.Triangles.IndexCount = indexBuffer->GetBufferDesc().NumElements;
			desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

			desc.Triangles.Transform3x4 = 0;

			blas->GeometryDescs.push_back(std::move(desc));

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.NumDescs = static_cast<uint32>(blas->GeometryDescs.size());
			inputs.pGeometryDescs = blas->GeometryDescs.data();

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
			device->LogicalDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

			blas->ScratchSize = ALIGN(prebuildInfo.ScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
			blas->ResultSize = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

			blas->ScratchBuffer = new D3D12Buffer(device, BufferDesc{
				.BufferUsage = BufferUsageFlag::UnorderedAccess,
				.Size = blas->ScratchSize,
				.bBindless = false,
				//.Name			= "BLAS Scratch Buffer",
				});

			blas->AccelerationStructure = new D3D12Buffer(device, BufferDesc{
				.BufferUsage = BufferUsageFlag::AccelerationStructure,
				.Size = blas->ResultSize,
				.bBindless = false,
				//.Name = "BLAS Result Buffer"
				});

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
			buildDesc.Inputs = inputs;
			buildDesc.SourceAccelerationStructureData = 0;
			buildDesc.ScratchAccelerationStructureData = blas->ScratchBuffer->GetGpuAddress();
			buildDesc.DestAccelerationStructureData = blas->AccelerationStructure->GetGpuAddress();

			auto* commandList = m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList;

			if (!commandList->IsOpen())
			{
				commandList->Open();
			}

			commandList->BuildRaytracingAccelerationStructure(buildDesc);

			D3D12_RESOURCE_BARRIER uavBarrier{};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = blas->AccelerationStructure->GetHandleRaw();
			uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			commandList->GetHandle()->ResourceBarrier(1, &uavBarrier);

			for (auto& mesh : pModel->Meshes)
			{
				mesh.RaytracingInstanceDesc.AccelerationStructure = blas->AccelerationStructure->GetGpuAddress();
				DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
				DirectX::XMStoreFloat3x4(&mesh.RaytracingInstanceDesc.Transform, matrix);
				Instances.push_back(mesh.RaytracingInstanceDesc);
			}

			blas->BuildDesc = buildDesc;



			BLASes.push_back(blas);
		}


	}

	D3D12BVH::D3D12BVH(D3D12RHI* pD3D12RHI)
		: m_D3D12RHI(pD3D12RHI)
	{
		TLAS = new D3D12TLAS(pD3D12RHI);
	}

	D3D12BVH::~D3D12BVH()
	{
		Release();
	}

	void D3D12BVH::Build(D3D12RHI* /* pD3D12RHI */, Scene* pActiveScene)
	{
		for (auto& model : pActiveScene->Models)
		{
			//TLAS->AddBLAS(model.get());
			TLAS->AddBLASPerMesh(model.get());
		}

		TLAS->Create(); 

		auto* commandList = m_D3D12RHI->Frames.at(BackBufferIndex).GraphicsCommandList;
		m_D3D12RHI->GraphicsQueue->Execute({ commandList });

		m_D3D12RHI->Wait();
		//m_D3D12RHI->Flush();

	}

	void D3D12BVH::Release()
	{
		for (auto blas : TLAS->BLASes)
		{
			delete blas;
		}

		delete TLAS;
	}

} // namespace Luden
