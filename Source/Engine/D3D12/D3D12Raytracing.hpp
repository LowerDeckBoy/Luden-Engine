#pragma once

#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"
#include "Graphics/Model.hpp"

namespace Luden
{
	class D3D12Device;

	class D3D12BLAS
	{
	public:
		//D3D12BLAS();

		void Create(D3D12Device* pDevice, StaticMesh& Mesh);

		D3D12_RAYTRACING_GEOMETRY_DESC Desc{};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Inputs{};

	}; // class D3D12BLAS

	class D3D12BVH
	{
	public:

		D3D12Resource* AccelerationStructure;
		D3D12Resource* ScratchBuffer;

		// instances
	}; // class D3D12BVH

} // namespace Luden
