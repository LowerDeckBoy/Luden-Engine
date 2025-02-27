#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include <Core/Logger.hpp>
#include <D3D12AgilitySDK/d3dx12/d3dx12_check_feature_support.h>
#include "RHI/Types.hpp"

namespace Luden
{
	D3D12Device::D3D12Device(D3D12Adapter* pParentAdapter)
		: ParentAdapter(pParentAdapter)
	{
		VERIFY_D3D12_RESULT(D3D12CreateDevice(ParentAdapter->Adapter, ParentAdapter->MinRequiredFeatureLevel, IID_PPV_ARGS(&LogicalDevice)));
		NAME_D3D12_OBJECT(LogicalDevice.Get(), "D3D12 Logical Device");

		if (Config::Get().bEnableDebugLayer)
		{
			VERIFY_D3D12_RESULT(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_DXGIDebug)));
			VERIFY_D3D12_RESULT(LogicalDevice->QueryInterface(IID_PPV_ARGS(&m_InfoQueue)));
			m_InfoQueue->RegisterMessageCallback(&DebugCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_DebugCallbackCookie);
		}

		QueryDeviceFeatures();
		
		ShaderResourceHeap	= new D3D12DescriptorHeap(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 65536);
		RenderTargetHeap	= new D3D12DescriptorHeap(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64);
		DepthStencilHeap	= new D3D12DescriptorHeap(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 8);

	}

	D3D12Device::~D3D12Device()
	{
		delete DepthStencilHeap;
		delete RenderTargetHeap;
		delete ShaderResourceHeap;

		SAFE_RELEASE(LogicalDevice);
		SAFE_RELEASE(m_InfoQueue);

		if (Config::Get().bEnableDebugLayer)
		{
			if (FAILED(m_DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL))))
			{
				LOG_WARNING("Failed to ReportLiveObjects!");
			}
			SAFE_RELEASE(m_DXGIDebug);
		}
	}
	
	void D3D12Device::QueryDeviceFeatures()
	{
		CD3DX12FeatureSupport features;
		features.Init(LogicalDevice.Get());

		auto shaderModel = features.HighestShaderModel();
		const auto string = ShaderModelToString(shaderModel);
		LOG_INFO("Highest HLSL Shader Model: {0}.", string);
		
	}

	uint32 D3D12Device::CreateBuffer(BufferDesc Desc)
	{
		uint32 handle = static_cast<uint32>(Buffers.size());
		
		Buffers.push_back(new D3D12Buffer(this, Desc));

		return handle;
	}

	uint32 D3D12Device::CreateConstantBuffer(void* pData, usize Size)
	{
		uint32 handle = static_cast<uint32>(ConstantBuffers.size());

		ConstantBuffers.push_back(new D3D12ConstantBuffer(this, pData, Size));

		return handle;
	}

	void D3D12Device::CreateShaderResourceView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 NumMips, uint32 Count)
	{
		const auto& resourceDesc = pResource->GetHandle()->GetDesc1();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = resourceDesc.Format;

		desc.ViewDimension				= D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels		= NumMips;
		desc.Texture2D.MostDetailedMip	= 0;
		desc.Texture2D.PlaneSlice		= 0;

		ShaderResourceHeap->Allocate(Descriptor, Count);

		LogicalDevice->CreateShaderResourceView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);

	}

	void D3D12Device::CreateShaderResourceView(D3D12Buffer* pBuffer)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping		= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension				= D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Format						= DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.FirstElement			= 0;
		srvDesc.Buffer.NumElements			= pBuffer->GetBufferDesc().NumElements;
		srvDesc.Buffer.StructureByteStride	= pBuffer->GetBufferDesc().Stride;
		srvDesc.Buffer.Flags				= D3D12_BUFFER_SRV_FLAG_NONE;

		ShaderResourceHeap->Allocate(pBuffer->ShaderResourceView);

		LogicalDevice->CreateShaderResourceView(pBuffer->GetHandleRaw(), &srvDesc, pBuffer->ShaderResourceView.CpuHandle);

	}

	void D3D12Device::CreateRenderTargetView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count)
	{
		const auto& resourceDesc = pResource->GetHandle()->GetDesc1();

		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = resourceDesc.Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		RenderTargetHeap->Allocate(Descriptor, Count);

		LogicalDevice->CreateRenderTargetView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);

	}

	void D3D12Device::CreateDepthStencilView(D3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Flags = D3D12_DSV_FLAG_NONE;
		desc.Format = Format;

		DepthStencilHeap->Allocate(Descriptor);

		LogicalDevice->CreateDepthStencilView(pResource->GetHandleRaw(), &desc, Descriptor.CpuHandle);
	}

	void D3D12Device::DebugCallback(
		D3D12_MESSAGE_CATEGORY	/* Category */,
		D3D12_MESSAGE_SEVERITY	Severity,
		D3D12_MESSAGE_ID		/* ID */,
		LPCSTR					pDescription,
		void*                   /* pContext */)
	{
		switch (Severity)
		{
		case D3D12_MESSAGE_SEVERITY_INFO:
			FALLTHROUGH;
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
			LOG_INFO("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_WARNING:
			LOG_WARNING("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_ERROR:
			LOG_ERROR("[D3D12] {}", pDescription);
			break;
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
			LOG_FATAL("[D3D12] {}", pDescription);
			break;
		}
	}
} // namespace Luden
