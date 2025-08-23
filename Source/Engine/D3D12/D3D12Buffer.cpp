#include "D3D12Device.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12Memory.hpp"
#include "D3D12Utility.hpp"
#include <Core/Math/Math.hpp>
#include <ranges>

namespace Luden
{
	D3D12Buffer::D3D12Buffer() = default;

	D3D12Buffer::D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc)
	{
		Create(pDevice, Desc);
	}

	D3D12Buffer::~D3D12Buffer()
	{
		Release();
	}
	
	void D3D12Buffer::Create(D3D12Device* pDevice, BufferDesc Desc)
	{
		if (Desc.Size == 0)
		{
			Desc.Size = static_cast<uint64>(Desc.NumElements * Desc.Stride);
		}

		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width              = Desc.Size;
		desc.Height             = 1;
		desc.DepthOrArraySize   = 1;
		desc.MipLevels          = 1;
		desc.Format             = DXGI_FORMAT_UNKNOWN;
		desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.SampleDesc         = { 1, 0 };

		D3D12_RESOURCE_STATES resourceState{};
		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		// Set properties according to destination buffer type.
		switch (Desc.BufferUsage)
		{
		case BufferUsageFlag::AccelerationStructure:
			desc.Flags		= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			//desc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceState	= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			break;
		case BufferUsageFlag::UnorderedAccess:
			desc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceState	= D3D12_RESOURCE_STATE_COMMON;
			break;
		case BufferUsageFlag::CopySrc:
			desc.Flags		= D3D12_RESOURCE_FLAG_NONE;
			resourceState	= D3D12_RESOURCE_STATE_COPY_SOURCE;
			break;
		case BufferUsageFlag::CopyDest:
			desc.Flags		= D3D12_RESOURCE_FLAG_NONE;
			resourceState	= D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		case BufferUsageFlag::Storage:
			desc.Flags		= D3D12_RESOURCE_FLAG_NONE;
			resourceState	= D3D12_RESOURCE_STATE_GENERIC_READ;
			allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
			break;
		default: 
			desc.Flags		= D3D12_RESOURCE_FLAG_NONE;
			resourceState	= D3D12_RESOURCE_STATE_COMMON;
			allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
			break;
		}

		VERIFY_D3D12_RESULT(pDevice->D3D12MemoryAllocator->CreateResource2(
			&allocationDesc,
			&desc,
			resourceState,
			nullptr,
			&m_ResourceAllocation,
			IID_PPV_ARGS(&m_Resource)));

		m_BufferDesc = Desc;

		if (Desc.bBindless)
		{
			pDevice->CreateShaderResourceView(this);   
		}

		if (!Desc.Name.empty())
		{
			SetDebugName(Desc.Name);
		}
	}

	D3D12_RESOURCE_DESC1 D3D12Buffer::CreateBufferDesc(uint64 Size, D3D12_RESOURCE_FLAGS Flags)
	{
		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Width = static_cast<uint64>(Size);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		// Note:
		// Setting D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT cause issue 
		// when building Acceleration Structures for Raytracing
		desc.Alignment = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = Flags;

		return desc;
	}

	D3D12DepthBuffer::D3D12DepthBuffer() = default;

	D3D12DepthBuffer::D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format, float DepthValue)
	{
		Create(pDevice, pViewport, Format, DepthValue);
	}
	
	D3D12DepthBuffer::~D3D12DepthBuffer()
	{

	}

	void D3D12DepthBuffer::Create(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format, float DepthValue)
	{
		Create(pDevice, 
			static_cast<uint32>(pViewport->Viewport.Width), 
			static_cast<uint32>(pViewport->Viewport.Height),
			Format,
			DepthValue);
	}

	void D3D12DepthBuffer::Create(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, float DepthValue)
	{
		m_Device = pDevice;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = Format;
		clearValue.DepthStencil.Depth = DepthValue;
		clearValue.DepthStencil.Stencil = 0;

		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width              = static_cast<uint64>(Width);
		desc.Height             = Height;
		desc.DepthOrArraySize   = 1;
		desc.MipLevels          = 1;
		desc.Format             = Format;
		desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.SampleDesc         = { 1, 0 };
		desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;// | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		m_Desc = desc;

		const auto& heapProperties = D3D::HeapPropertiesDefault();

		VERIFY_D3D12_RESULT(m_Device->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			nullptr,
			IID_PPV_ARGS(&m_Resource)));

		m_Device->CreateDepthStencilView(this, DepthStencilHandle, Format);
		D3D12Resource::SetResourceState(D3D12_RESOURCE_STATE_DEPTH_WRITE);
		D3D12_DEPTH_STENCIL_VIEW_DESC readDesc{};
		readDesc.Format = Format;
		readDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

		if (GetDesc().SampleDesc.Count == 1)
		{
			readDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			readDesc.Texture2D.MipSlice = 0;
		}
		else
		{
			readDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		}

		m_Device->DepthStencilHeap->Allocate(DepthReadHandle);
		//m_Device->LogicalDevice->CreateDepthStencilView(GetHandleRaw(), &readDesc, DepthReadHandle.CpuHandle);

	}

	void D3D12DepthBuffer::Resize(uint32 Width, uint32 Height, float DepthValue)
	{
		if (IsValid())
		{
			Release();
		}

		Create(m_Device, Width, Height, m_Format, DepthValue);
	}

	D3D12ConstantBuffer::D3D12ConstantBuffer(D3D12Device* pDevice, void* pData, usize Size)
	{
		Create(pDevice, pData, Size);
	}

	D3D12ConstantBuffer::~D3D12ConstantBuffer()
	{
		Release();
	}

	void D3D12ConstantBuffer::Create(D3D12Device* pDevice, void* pData, usize Size)
	{
		m_Device = pDevice;

		// Align data to 256 bytes
		m_Size = ALIGN(Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		uint32 numBackBuffers = Config::Get().NumBackBuffers;

		m_Data.resize(numBackBuffers);
		m_Buffers.resize(numBackBuffers);
		pDataBegin.resize(numBackBuffers);

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Format             = DXGI_FORMAT_UNKNOWN;
		desc.Width              = static_cast<uint64>(Size);
		desc.Height             = 1;
		desc.DepthOrArraySize   = 1;
		desc.MipLevels          = 1;
		desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc         = { 1, 0 };
		desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

		auto heapProperties = D3D::HeapPropertiesUpload();

		for (uint32 i = 0; i < numBackBuffers; i++)
		{
			m_Device->LogicalDevice->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&m_Buffers.at(i)));

			m_Data.at(i) = pData;

			// Persistent mapping
			const D3D12_RANGE readRange(0, 0);
			VERIFY_D3D12_RESULT(m_Buffers.at(i)->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin.at(i))));
			std::memcpy(pDataBegin.at(i), &pData, m_Size);
		}
	}

	void D3D12ConstantBuffer::Update(void* pUpdate)
	{
		m_Data.at(BackBufferIndex) = pUpdate;
		std::memcpy(pDataBegin.at(BackBufferIndex), pUpdate, m_Size);
	}

	Ref<ID3D12Resource>& D3D12ConstantBuffer::GetBuffer()
	{
		return m_Buffers.at(BackBufferIndex);
	}

	void D3D12ConstantBuffer::Release()
	{
		for (auto& buffer : m_Buffers)
		{
			SAFE_RELEASE(buffer);
		}
	}

} // namespace Luden
