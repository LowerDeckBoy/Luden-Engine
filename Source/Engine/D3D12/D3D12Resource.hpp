#pragma once

#include <Core/RefPtr.hpp>
#include <Core/String.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <D3D12MemoryAllocator/D3D12MemAlloc.h>

namespace Luden
{
	class D3D12Device;

	// Wrapper for ID3D12Resource2
	enum class ResourceUsage
	{
		Uninitialized,

		Texture1D,
		Texture2D,
		Texture3D,

		Buffer,
		AccelerationStructure,
	};

	// Wrapper for ID3D12Resource2 object.
	// Records Resource current state and holds resource allocation.
	class D3D12Resource
	{
	public:
		D3D12Resource() = default;
		D3D12Resource(D3D12Device* pDevice, const D3D12_RESOURCE_DESC1& Desc, D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_PROPERTIES HeapProperties, ResourceUsage UsageFlag);
		~D3D12Resource() = default;

		void Create(D3D12Device* pDevice, const D3D12_RESOURCE_DESC1& Desc, D3D12_RESOURCE_STATES InitialState, D3D12_HEAP_PROPERTIES HeapProperties, ResourceUsage UsageFlag);

		Ref<ID3D12Resource2>& GetHandle()
		{
			return m_Resource;
		}

		ID3D12Resource2* GetHandleRaw()
		{
			return m_Resource.Get();
		}

		D3D12_RESOURCE_STATES& GetCurrentState()
		{
			return m_CurrentState;
		}

		// Release Resource, it's Allocation and clear all states to default ones.
		void Release();

		void SetResourceState(D3D12_RESOURCE_STATES State);

		void SetDebugName(std::string_view DebugName);

		bool IsValid() const
		{
			return m_Resource;
		}

		ResourceUsage GetResourceUsageFlag() const
		{
			return m_UsageFlag;
		}

	protected:
		Ref<ID3D12Resource2>		m_Resource;
		Ref<ID3D12Resource2>		m_UploadResource;
		Ref<D3D12MA::Allocation>	m_ResourceAllocation;

		D3D12_RESOURCE_STATES		m_CurrentState = D3D12_RESOURCE_STATE_GENERIC_READ;
		D3D12_RESOURCE_DESC1		m_Desc{};

		ResourceUsage m_UsageFlag = ResourceUsage::Uninitialized;

		D3D12Device* m_ParentDevice = nullptr;

	};
} // namespace Luden
