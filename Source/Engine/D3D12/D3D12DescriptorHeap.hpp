#pragma once

#include <Core/RefPtr.hpp>
#include <Core/Types.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <D3D12AgilitySDK/d3dx12/d3dx12.h>

namespace Luden
{
	class D3D12Device;

	static constexpr uint32 INVALID_DESCRIPTOR_INDEX = 0xffffffff;

	enum class DescriptorTypeFlag
	{
		ShaderResource	= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		RenderTarget	= D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DepthStencil	= D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		Sampler			= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
	};
	
	class D3D12Descriptor
	{
		friend class D3D12DescriptorHeap;
	public:
		D3D12Descriptor() = default;
		D3D12Descriptor(const D3D12Descriptor& Other)
		{
			CpuHandle	= Other.CpuHandle;
			GpuHandle	= Other.GpuHandle;
			Index		= Other.Index;
		}

		//D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		//D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle;

		// Index of this Descriptor inside of parent Heap.
		uint32 Index = INVALID_DESCRIPTOR_INDEX;

		constexpr bool IsValid() const
		{
			return CpuHandle.ptr != 0 && Index != INVALID_DESCRIPTOR_INDEX;
		}

		constexpr bool IsShaderVisible() const
		{
			return bIsShaderVisible; // GpuHandle.ptr != 0;
		}
		
	private:
		D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType{};
		bool bIsShaderVisible = false;

	};

	class D3D12DescriptorHeap
	{
		friend class D3D12Descriptor;
	public:
		D3D12DescriptorHeap() = default;
		D3D12DescriptorHeap(D3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType, uint32 MaxCapacity);
		~D3D12DescriptorHeap();

		Ref<ID3D12DescriptorHeap>&	GetHandle()		{ return m_DescriptorHeap;		 }
		ID3D12DescriptorHeap*		GetHandleRaw()	{ return m_DescriptorHeap.Get(); }

		void Create(D3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType, uint32 MaxCapacity, bool bShaderVisible);

		void Allocate(D3D12Descriptor& Descriptor, uint32 Count = 1);

		void Override(D3D12Descriptor& Descriptor);

		void Reset();

		uint32 GetIndex(D3D12Descriptor& Descriptor);
		uint32 GetIndexFromOffset(D3D12Descriptor& Descriptor, uint32 Offset);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuStartHandle() const;
		

		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuStartHandle() const;

		uint64 GetCpuStartHandlePtr() const;
		
		uint64 GetGpuStartHandlePtr() const;
		

		uint32 GetDescriptorIncrementSize() const
		{
			return m_DescriptorIncrementSize;
		}

	private:
		Ref<ID3D12DescriptorHeap> m_DescriptorHeap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorType{};

		uint32 m_MaxCapacity = 0;
		uint32 m_CurrentAllocations = 0;

		uint32 m_DescriptorIncrementSize = 0;

		uint64 m_AvailableCpuPtr = 0;
		uint64 m_AvailableGpuPtr = 0;

		bool bIsShaderVisible = false;

		constexpr inline bool CanAllocate() const
		{
			return m_CurrentAllocations < m_MaxCapacity;
		}
	};

	// TEST
	// Single class to manage all Descriptor Heaps:
	// - CPU visible only (non-staging) for Descriptor allocations.
	// - GPU only (staging)
	//class D3D12DescriptorHeapManager
	//{
	//public:
	//	D3D12DescriptorHeapManager(D3D12Device* pDevice);
	//	~D3D12DescriptorHeapManager();
	//
	//	// CPU visible only Heaps
	//	D3D12DescriptorHeap ShaderResourceHeap;
	//	D3D12DescriptorHeap RenderTargetHeap;
	//	D3D12DescriptorHeap DepthStencilHeap;
	//
	//	// Staging Heaps
	//	// One per BackBuffer
	//	std::vector<D3D12DescriptorHeap> StagingShaderResourcesHeaps;
	//	std::vector<D3D12DescriptorHeap> StagingRenderTargetHeap;
	//	std::vector<D3D12DescriptorHeap> StagingDepthStencilHeap;
	//
	//private:
	//
	//};

} // namespace Luden
