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
        desc.Alignment          = 0;
        desc.SampleDesc         = { 1, 0 };
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        if (Desc.BufferUsage == BufferUsageFlag::AccelerationStructure)
        {
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        auto heapProperties = D3D::HeapPropertiesDefault();

        auto resourceFlag = (Desc.BufferUsage == BufferUsageFlag::AccelerationStructure) ?
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE : D3D12_RESOURCE_STATE_COMMON;

        VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            resourceFlag,
            nullptr,
            nullptr,
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

    D3D12DepthBuffer::D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format)
    {
        Create(pDevice, pViewport, Format);
    }
    
    D3D12DepthBuffer::~D3D12DepthBuffer()
    {

    }

    void D3D12DepthBuffer::Create(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format)
    {
        Create(pDevice, 
            static_cast<uint32>(pViewport->Viewport.Width), 
            static_cast<uint32>(pViewport->Viewport.Height),
            Format);
    }

    void D3D12DepthBuffer::Create(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format)
    {
        m_Device = pDevice;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = Format;
        clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
        clearValue.DepthStencil.Stencil = 0;

        D3D12_RESOURCE_DESC1 desc{};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = static_cast<uint64>(Width);
        desc.Height             = Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = Format;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        //desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        desc.SampleDesc         = { 1, 0 };
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

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

    }

    void D3D12DepthBuffer::Resize(uint32 Width, uint32 Height)
    {
        if (IsValid())
        {
            Release();
        }

        Create(m_Device, Width, Height, m_Format);
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
