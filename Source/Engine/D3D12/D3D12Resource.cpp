#include "D3D12Resource.hpp"
#include "D3D12Utility.hpp"

namespace Luden
{
    D3D12Resource::~D3D12Resource()
    {
        Release();
        m_Desc = {};
    }

    uint64 D3D12Resource::GetGpuAddress()
    {
        return GetHandleRaw()->GetGPUVirtualAddress();
    }

    void D3D12Resource::Release()
    {
        SAFE_RELEASE(m_Resource);
    }

    void D3D12Resource::SetResourceState(D3D12_RESOURCE_STATES State)
    {
        m_CurrentState = State;
    }

    void D3D12Resource::SetDebugName(std::string_view Name)
    {
        NAME_D3D12_OBJECT(m_Resource.Get(), Name);

        DebugName = Name;
    }
} // namespace Luden
