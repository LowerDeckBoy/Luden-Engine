#include "D3D12RHI.hpp"

namespace Luden
{
	D3D12RHI::D3D12RHI(Platform::Window* pParentWindow)
		: m_ParentWindow(pParentWindow)
	{

		Adapter = new D3D12Adapter();
		Device = new D3D12Device(Adapter);

		GraphicsQueue = new D3D12CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		SwapChain = new D3D12SwapChain(Device, GraphicsQueue, m_ParentWindow);

	}

	D3D12RHI::~D3D12RHI()
	{
		delete GraphicsQueue;
		delete SwapChain;
		delete Adapter;
		delete Device;
	}
} // namespace Luden
