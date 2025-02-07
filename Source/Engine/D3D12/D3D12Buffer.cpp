#include "D3D12DescriptorHeap.hpp"
#include "D3D12RHI.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12Memory.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
    D3D12Buffer::D3D12Buffer() = default;

    D3D12Buffer::D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc)
    {
        Create(pDevice, Desc);
    }

    D3D12Buffer::~D3D12Buffer()
    {
        
    }
    
    void D3D12Buffer::Create(D3D12Device* pDevice, BufferDesc Desc)
    {
        D3D12_RESOURCE_DESC1 desc{};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width              = Desc.Size;
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        desc.SampleDesc         = { 1, 0 };

        const auto& heapProperties = D3D::HeapPropertiesDefault();

        VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
            &heapProperties,
            D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            nullptr,
            IID_PPV_ARGS(&m_Resource)));

        if (Desc.bBindless)
        {

        }
    }

    D3D12DepthBuffer::D3D12DepthBuffer() = default;

    D3D12DepthBuffer::D3D12DepthBuffer(D3D12RHI* pD3D12RHI, D3D12Viewport* pViewport, DXGI_FORMAT Format)
    {
        Create(pD3D12RHI, pViewport, Format);
    }
    
    D3D12DepthBuffer::~D3D12DepthBuffer()
    {

    }

    void D3D12DepthBuffer::Create(D3D12RHI* pD3D12RHI, D3D12Viewport* pViewport, DXGI_FORMAT Format)
    {
        Create(pD3D12RHI, 
            static_cast<uint32>(pViewport->Viewport.Width), 
            static_cast<uint32>(pViewport->Viewport.Height),
            Format);
    }

    void D3D12DepthBuffer::Create(D3D12RHI* pD3D12RHI, uint32 Width, uint32 Height, DXGI_FORMAT Format)
    {
        m_D3D12RHI = pD3D12RHI;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = m_Format;
        clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
        clearValue.DepthStencil.Stencil = 0;

        D3D12_RESOURCE_DESC1 desc{};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = static_cast<uint64>(Width);
        desc.Height             = Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Alignment          = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        desc.SampleDesc         = { 1, 0 };
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        const auto& heapProperties = D3D::HeapPropertiesDefault();

        VERIFY_D3D12_RESULT(pD3D12RHI->Device->LogicalDevice->CreateCommittedResource2(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            nullptr,
            IID_PPV_ARGS(&m_Resource)));


        pD3D12RHI->DepthStencilHeap->Allocate(DepthStencilHandle);
        pD3D12RHI->CreateDepthStencilView(this, DepthStencilHandle, Format);
    }

    void D3D12DepthBuffer::Resize(uint32 Width, uint32 Height)
    {
        if (IsValid())
        {
            Release();
        }

        Create(m_D3D12RHI, Width, Height, m_Format);
    }

} // namespace Luden
