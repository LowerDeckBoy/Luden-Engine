#pragma once

#include "D3D12Resource.hpp"
#include <Core/Types.hpp>

namespace Luden
{
	class D3D12RHI;
	class D3D12Device;
	class D3D12Descriptor;
	class D3D12Viewport;

	enum class BufferUsageFlag
	{
		Vertex,
		Index,
		Structured,
		Constant,
		Copy,
		AccelerationStructure
	};

	struct BufferDesc
	{
		BufferUsageFlag BufferUsage;
		void*	pData;
		uint32	NumElements;
		uint32	Stride;

		// Size = NumElements * Stride
		// Not need to set manually.
		uint64	Size;
		
		// By default all buffers are considered for bindless usage.
		bool bBindless = true;

		// Optional
		std::string_view Name = "";
	};

	class D3D12Buffer : public D3D12Resource
	{
	public:
		D3D12Buffer();
		D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc);
		~D3D12Buffer();

		void Create(D3D12Device* pDevice, BufferDesc Desc);

	private:
		BufferDesc m_BufferDesc{};
		
	};

	template<typename T>
	class D3D12ConstantBuffer
	{
	public:

	private:

	};

	class D3D12DepthBuffer : public D3D12Resource
	{
	public:
		D3D12DepthBuffer();
		D3D12DepthBuffer(D3D12RHI* pD3D12RHI, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		~D3D12DepthBuffer();

		void Create(D3D12RHI* pD3D12RHI, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		void Create(D3D12RHI* pD3D12RHI, uint32 Width, uint32 Height, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);

		void Resize(uint32 Width, uint32 Height);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor DepthStencilHandle;
		
	private:
		D3D12RHI* m_D3D12RHI = nullptr;
		DXGI_FORMAT m_Format = DXGI_FORMAT_D32_FLOAT;

	};
} // namespace Luden
