#include "D3D12Device.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>

namespace Luden
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType, uint32 MaxCapacity)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NodeMask		= pDevice->NodeMask;
		desc.Type			= DescriptorType;
		desc.NumDescriptors = MaxCapacity;
		
		desc.Flags = (DescriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || DescriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		VERIFY_D3D12_RESULT(pDevice->Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DescriptorHeap)));

		m_MaxCapacity = MaxCapacity;
		
		m_DescriptorIncrementSize = pDevice->Device->GetDescriptorHandleIncrementSize(DescriptorType);

		switch (DescriptorType)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 ShaderResource Descriptor Heap");
			bIsShaderVisible = true;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 Sampler Descriptor Heap");
			bIsShaderVisible = true;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 RenderTarget Descriptor Heap");
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 DepthStencil Descriptor Heap");
			break;
		}

		m_DescriptorType = DescriptorType;

		m_AvailableCpuPtr = static_cast<uint64>(GetCpuStartHandlePtr()); //  + m_DescriptorIncrementSize

		if (bIsShaderVisible)
		{
			m_AvailableGpuPtr = static_cast<uint64>(GetGpuStartHandlePtr());
		}
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
	}

	void D3D12DescriptorHeap::Create(D3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType, uint32 MaxCapacity, bool /* bShaderVisible */)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NodeMask		= pDevice->NodeMask;
		desc.Type			= DescriptorType;
		desc.NumDescriptors = MaxCapacity;

		desc.Flags =
			(DescriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || DescriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		VERIFY_D3D12_RESULT(pDevice->Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DescriptorHeap)));

		m_MaxCapacity = MaxCapacity;

		m_DescriptorIncrementSize = pDevice->Device->GetDescriptorHandleIncrementSize(DescriptorType);

		switch (DescriptorType)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 ShaderResource Descriptor Heap");
			bIsShaderVisible = true;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 Sampler Descriptor Heap");
			bIsShaderVisible = true;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 RenderTarget Descriptor Heap");
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			NAME_D3D12_OBJECT(m_DescriptorHeap.Get(), "D3D12 DepthStencil Descriptor Heap");
			break;
		}

		m_DescriptorType = DescriptorType;

		m_AvailableCpuPtr = static_cast<uint64>(GetCpuStartHandlePtr()); //  + m_DescriptorIncrementSize

		if (bIsShaderVisible)
		{
			m_AvailableGpuPtr = static_cast<uint64>(GetGpuStartHandlePtr());
		}
	}

	void D3D12DescriptorHeap::Allocate(D3D12Descriptor& Descriptor, uint32 Count)
	{
		if (!CanAllocate())
		{
			LOG_WARNING("Current Descriptor is full and can't allocated no more!");

			return;
		}

		if (Descriptor.IsValid())
		{
			Override(Descriptor);

			return;
		}

		Descriptor.DescriptorType = m_DescriptorType;

		Descriptor.CpuHandle = (D3D12_CPU_DESCRIPTOR_HANDLE)(m_AvailableCpuPtr + (static_cast<uint64>(Count * m_DescriptorIncrementSize)));
		Descriptor.Index = GetIndex(Descriptor);

		if (bIsShaderVisible)
		{
			Descriptor.bIsShaderVisible = true;

			Descriptor.GpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE)(m_AvailableGpuPtr + (static_cast<uint64>(Count * m_DescriptorIncrementSize)));
		}
	}

	void D3D12DescriptorHeap::Override(D3D12Descriptor& Descriptor)
	{
		uint32 offset = GetIndex(Descriptor);

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = (D3D12_CPU_DESCRIPTOR_HANDLE)(GetCpuStartHandle().ptr + (offset * m_DescriptorIncrementSize));
		Descriptor.CpuHandle = cpuHandle;

		if (bIsShaderVisible)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE)(GetGpuStartHandle().ptr + (offset * m_DescriptorIncrementSize));
			Descriptor.GpuHandle = gpuHandle;
		}
	}

	void D3D12DescriptorHeap::Reset()
	{
		m_AvailableCpuPtr = GetCpuStartHandlePtr();

		if (bIsShaderVisible)
		{
			m_AvailableGpuPtr = GetGpuStartHandlePtr();
		}

		m_CurrentAllocations = 0;

	}

	uint32 D3D12DescriptorHeap::GetIndex(D3D12Descriptor& Descriptor)
	{
		return static_cast<uint32>((Descriptor.CpuHandle.ptr - GetCpuStartHandlePtr()) / m_DescriptorIncrementSize);
	}

	uint32 D3D12DescriptorHeap::GetIndexFromOffset(D3D12Descriptor& Descriptor, uint32 Offset)
	{
		return static_cast<uint32>((Descriptor.CpuHandle.ptr + (usize)(Offset * m_DescriptorIncrementSize) - GetCpuStartHandlePtr()) / m_DescriptorIncrementSize);
	}
} // namespace Luden
	