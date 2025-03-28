#pragma once

/*
	Some helpers for D3D12 related properties.
*/

#include <D3D12AgilitySDK/d3dx12/d3dx12_resource_helpers.h>

#define BYTES_TO_MEGABYTES(Bytes) (Bytes / 1024 / 1024)
#define BYTES_TO_GIGABYTES(Bytes) (BYTES_TO_MEGABYTES(Bytes) / 1024)

namespace Luden::D3D
{
	inline D3D12_HEAP_PROPERTIES HeapPropertiesDefault()
	{
		return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	}

	inline D3D12_HEAP_PROPERTIES HeapPropertiesUpload()
	{
		return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	}

	inline D3D12_HEAP_PROPERTIES HeapPropertiesReadback()
	{
		return CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	}
} // namespace Luden::D3D
