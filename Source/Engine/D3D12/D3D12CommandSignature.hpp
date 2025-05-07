#pragma once

#include <D3D12AgilitySDK/d3d12.h>
#include <Core/RefPtr.hpp>
#include <vector>

#include "D3D12Buffer.hpp"

namespace Luden
{
	class D3D12Device;
	class D3D12RootSignature;

	struct FDispatchMeshCommand
	{
		D3D12_DISPATCH_MESH_ARGUMENTS Argument;
	};

	class D3D12CommandSignature
	{
	public:
		D3D12CommandSignature() = default;
		~D3D12CommandSignature()
		{
			Release();
		}

		HRESULT Build(D3D12Device* pDevice, D3D12RootSignature* pRootSignature);
		
		void CreateCommandsBuffer();

		Ref<ID3D12CommandSignature>&	GetHandle()		{ return m_CommandSignature;		}
		ID3D12CommandSignature*			GetHandleRaw()	{ return m_CommandSignature.Get();	}

		void AddDrawIndexedCommand();
		void AddDispatchMeshCommand();
		void AddDispatchRaysCommand();
		void AddConstantsCommand(uint32 NumConstants, uint32 RootIndex, uint32 Offset = 0);
		void AddConstantsBufferViewCommand(uint32 RootIndex);

		void Release();

		void SetDebugName(std::string_view Name);

		std::vector<D3D12_INDIRECT_ARGUMENT_DESC> ArgumentDescs;
		uint32 ByteStride = 0;	

		D3D12Buffer* GetCommandsBuffer() { return m_CommandsBuffer; }

	private:
		Ref<ID3D12CommandSignature> m_CommandSignature;
		
		D3D12Buffer* m_CommandsBuffer;

		D3D12Device* m_ParentDevice		= nullptr;
	};
} // namespace Luden
