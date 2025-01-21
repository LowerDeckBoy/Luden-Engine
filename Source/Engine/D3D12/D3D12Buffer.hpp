#pragma once

#include "D3D12Resource.hpp"
#include <Core/Types.hpp>

namespace Luden
{
	class D3D12Device;

	enum class BufferUsageFlag
	{
		Vertex,
		Index,
		Structured,
		Constant,
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
} // namespace Luden
