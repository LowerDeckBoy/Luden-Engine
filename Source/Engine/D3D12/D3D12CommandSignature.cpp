#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12CommandSignature.hpp"
#include "D3D12Utility.hpp"
#include <Core/Assert.hpp>

namespace Luden
{
	HRESULT D3D12CommandSignature::Build(D3D12Device* pDevice, D3D12RootSignature* pRootSignature)
	{
		ASSERT(ArgumentDescs.size() > 0 && ByteStride > 0);

		m_ParentDevice = pDevice;

		D3D12_COMMAND_SIGNATURE_DESC desc{};
		desc.NodeMask			= pDevice->NodeMask;
		desc.pArgumentDescs		= ArgumentDescs.data();
		desc.NumArgumentDescs	= static_cast<uint32>(ArgumentDescs.size());
		desc.ByteStride			= ByteStride;

		return pDevice->LogicalDevice->CreateCommandSignature(&desc, pRootSignature->GetHandleRaw(), IID_PPV_ARGS(&m_CommandSignature));
	}

	void D3D12CommandSignature::CreateCommandsBuffer()
	{
	}

	void D3D12CommandSignature::AddDrawIndexedCommand()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		
		ByteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);

		ArgumentDescs.push_back(argument);
	}

	void D3D12CommandSignature::AddDispatchMeshCommand()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

		ByteStride += sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);

		ArgumentDescs.push_back(argument);
	}

	void D3D12CommandSignature::AddDispatchRaysCommand()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS;

		ByteStride += sizeof(D3D12_DISPATCH_ARGUMENTS);

		ArgumentDescs.push_back(argument);
	}

	void D3D12CommandSignature::AddConstantsCommand(uint32 NumConstants, uint32 RootIndex, uint32 Offset)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argument.Constant.Num32BitValuesToSet = NumConstants;
		argument.Constant.RootParameterIndex = RootIndex;
		argument.Constant.DestOffsetIn32BitValues = Offset;

		ByteStride += sizeof(uint32) * NumConstants;

		ArgumentDescs.push_back(argument);
	}

	void D3D12CommandSignature::AddConstantsBufferViewCommand(uint32 RootIndex)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		argument.ConstantBufferView.RootParameterIndex = RootIndex;

		ByteStride += sizeof(uint64);

		ArgumentDescs.push_back(argument);
	}

	void D3D12CommandSignature::Release()
	{
		m_CommandSignature.Release();

		ArgumentDescs.clear();
		ArgumentDescs.shrink_to_fit();

		ByteStride = 0;
	}

	void D3D12CommandSignature::SetDebugName(std::string_view Name)
	{
		NAME_D3D12_OBJECT(m_CommandSignature.Get(), Name);
	}
} // namespace Luden
