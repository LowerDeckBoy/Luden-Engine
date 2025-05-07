#pragma once

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include <Core/Types.hpp>
#include <vector>

namespace Luden
{
	class D3D12Device;
	class D3D12Viewport;

	enum class BufferUsageFlag
	{
		Vertex,
		Index,
		Structured,
		Constant,
		CopySrc,
		CopyDest,
		IndirectArgument,
		AccelerationStructure,
	};

	struct BufferDesc
	{
		BufferUsageFlag BufferUsage;
		void*	Data;
		uint32	NumElements;
		uint32	Stride;

		// If Size is not specified, then Size = NumElements * Stride.
		uint64	Size = 0;
		
		// By default all buffers are considered for bindless usage.
		bool	bBindless = true;

		// Optional
		std::string Name = "";
	};

	class D3D12Buffer : public D3D12Resource
	{
	public:
		D3D12Buffer();
		D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc);
		~D3D12Buffer();

		void Create(D3D12Device* pDevice, BufferDesc Desc);

		D3D12Descriptor ShaderResourceView;

		BufferDesc& GetBufferDesc()
		{
			return m_BufferDesc;
		}

		static D3D12_RESOURCE_DESC1 CreateBufferDesc(uint64 Size, D3D12_RESOURCE_FLAGS Flags);

	private:
		BufferDesc m_BufferDesc{};
		
	};

	
	class D3D12ConstantBuffer
	{
	public:
		D3D12ConstantBuffer() = default;
		D3D12ConstantBuffer(D3D12Device* pDevice, void* pData, usize Size);
		~D3D12ConstantBuffer();

		void Create(D3D12Device* pDevice, void* pData, usize Size);

		// TODO: needs improvement
		void Update(void* pUpdate);

		Ref<ID3D12Resource>& GetBuffer();

		void Release();

		std::vector<uint8_t*> pDataBegin{};

	private:
		std::vector<Ref<ID3D12Resource>> m_Buffers;
		std::vector<void*> m_Data{};
		usize m_Size = 0;
	
		D3D12Device* m_Device;


	};

	class D3D12DepthBuffer : public D3D12Resource
	{
	public:
		D3D12DepthBuffer();
		D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		~D3D12DepthBuffer();

		void Create(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		void Create(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		void Resize(uint32 Width, uint32 Height);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor DepthStencilHandle;
		
	private:
		D3D12Device* m_Device = nullptr;
		DXGI_FORMAT m_Format = DXGI_FORMAT_D32_FLOAT;

	};

	inline D3D12_INDEX_BUFFER_VIEW GetIndexView(D3D12Buffer* pBuffer)
	{
		return D3D12_INDEX_BUFFER_VIEW{
			.BufferLocation = pBuffer->GetGpuAddress(),
			.SizeInBytes	= static_cast<uint32>(pBuffer->GetBufferDesc().Size),
			.Format			= DXGI_FORMAT_R32_UINT
		};
	}
} // namespace Luden
