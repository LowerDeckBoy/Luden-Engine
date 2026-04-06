#pragma once

//#include <vector>
#include "D3D12Buffer.hpp"

namespace Luden
{
	class D3D12Device;
	class D3D12RootSignature;

	struct FDispatchMeshCommand
	{
		D3D12_DISPATCH_MESH_ARGUMENTS Argument;

		uint32 VertexBufferIndex;
		uint32 MeshletBufferIndex;
		uint32 MeshletVerticesIndex;
		uint32 MeshletTrianglesIndex;
		uint32 MeshletBoundsBufferIndex;

		uint32 TransformsBufferIndex;
		uint32 MaterialsBufferIndex;
		uint32 MaterialID;
		uint32 TransformID;

		float  NearZ;
		float  FarZ;
	};

	class D3D12CommandSignature
	{
	public:
		D3D12CommandSignature() = default;
		D3D12CommandSignature(D3D12Device* pDevice)
			: m_ParentDevice(pDevice) {}
		~D3D12CommandSignature();

		HRESULT Build(D3D12Device* pDevice, D3D12RootSignature* pRootSignature);
		
		void CreateCommandsBuffer(BufferDesc Desc);

		ID3D12CommandSignature*	GetHandleRaw() { return m_CommandSignature.Get(); }

		void AddDrawIndexedCommand();
		void AddDispatchMeshCommand();
		void AddDispatchRaysCommand();
		void AddConstantsCommand(uint32 NumConstants, uint32 RootIndex, uint32 Offset = 0);
		void AddConstantsBufferViewCommand(uint32 RootIndex);

		void AddDispatchMeshArgument(uint32 DispatchCountX, uint32 DispatchCountY = 1, uint32 DispatchCountZ = 1);

		void Release();

		void SetDebugName(std::string_view Name);

		std::vector<D3D12_INDIRECT_ARGUMENT_DESC> ArgumentDescs;
		uint32 ByteStride = 0;	

		D3D12Buffer* GetCommandsBuffer() { return m_CommandsBuffer; }

	private:
		Ref<ID3D12CommandSignature> m_CommandSignature;
		D3D12Buffer* m_CommandsBuffer = nullptr;

		D3D12Device* m_ParentDevice	= nullptr;

	};
} // namespace Luden
