#include "D3D12Raytracing.hpp"

namespace Luden
{
    void D3D12BLAS::Create(D3D12Device* pDevice, StaticMesh& Mesh)
    {
        auto& vertexBuffer  = pDevice->Buffers.at(Mesh.VertexBuffer);
        auto& indexBuffer   = pDevice->Buffers.at(Mesh.IndexBuffer);

        Desc.Type  = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        Desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

        Desc.Triangles.VertexBuffer.StartAddress  = vertexBuffer->GetGpuAddress();
        Desc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetBufferDesc().Stride;
        Desc.Triangles.VertexCount  = vertexBuffer->GetBufferDesc().NumElements;
        Desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

        Desc.Triangles.IndexBuffer  = indexBuffer->GetGpuAddress();
        Desc.Triangles.IndexCount   = indexBuffer->GetBufferDesc().NumElements;
        Desc.Triangles.IndexFormat  = DXGI_FORMAT_R32_UINT;

        Desc.Triangles.Transform3x4 = 0;

        Inputs.Type  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
        Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        Inputs.pGeometryDescs = &Desc;
        Inputs.NumDescs = 1;


    }
} // namespace Luden
