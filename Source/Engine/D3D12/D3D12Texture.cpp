#include "D3D12RHI.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Memory.hpp"

#include <DirectXTex.h>

namespace Luden
{
	D3D12Texture::D3D12Texture(D3D12Device* pDevice, TextureDesc Desc)
		: m_TextureDesc(Desc)
	{
		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = Desc.Format;
		desc.Width = static_cast<uint64>(Desc.Width);
		desc.Height = Desc.Height;
		desc.DepthOrArraySize = Desc.DepthOrArray;
		desc.MipLevels = Desc.NumMips;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.SampleDesc = { 1, 0 };

		const auto& heapProperties = D3D::HeapPropertiesDefault();
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_Resource)
		));

		SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);

		pDevice->CreateShaderResourceView(this, ShaderResourceHandle, desc.MipLevels, 1);

	}

	D3D12Texture::D3D12Texture(D3D12Device* pDevice, TextureDesc Desc, Filepath Path)
		: m_TextureDesc(Desc)
	{
		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format				= Desc.Format;
		desc.Width				= static_cast<uint64>(Desc.Width);
		desc.Height				= Desc.Height;
		desc.DepthOrArraySize	= Desc.DepthOrArray;
		desc.MipLevels			= Desc.NumMips;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Alignment			= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.SampleDesc			= { 1, 0 };

		const auto& heapProperties = D3D::HeapPropertiesDefault();
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_Resource)
		)); 
		
		SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);

		pDevice->CreateShaderResourceView(this, ShaderResourceHandle, desc.MipLevels, 1);

	}

	D3D12Texture::~D3D12Texture()
	{
		Release();
	}

	void D3D12Texture::Create(D3D12Device* pDevice, TextureDesc Desc)
	{
		if (!m_Resource)
		{
			SAFE_RELEASE(m_Resource);
		}
		
		m_TextureDesc = std::move(Desc);

		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format				= Desc.Format;
		desc.Width				= static_cast<uint64>(Desc.Width);
		desc.Height				= Desc.Height;
		desc.DepthOrArraySize	= Desc.DepthOrArray;
		desc.MipLevels			= Desc.NumMips;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Alignment			= 0;
		desc.SampleDesc			= { 1, 0 };

		const auto& heapProperties = D3D::HeapPropertiesDefault();
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_Resource)
		));

		SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
		D3D12Resource::m_Desc = desc;

		pDevice->CreateShaderResourceView(this, ShaderResourceHandle, desc.MipLevels, 1);

	}

	D3D12RenderTexture::D3D12RenderTexture(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, std::string_view Name)
	{
		Create(pDevice, Width, Height, Format, Name);
	}

	D3D12RenderTexture::~D3D12RenderTexture()
	{
		Release();
	}

	void D3D12RenderTexture::Create(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, std::string_view Name)
	{
		if (IsValid())
		{
			Release();
		}

		m_Device = pDevice;
		m_TextureDesc.Width = Width;
		m_TextureDesc.Height = Height;
		m_TextureDesc.Format = Format;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = DefaultClearColor.at(0);
		clearValue.Color[1] = DefaultClearColor.at(1);
		clearValue.Color[2] = DefaultClearColor.at(2);
		clearValue.Color[3] = DefaultClearColor.at(3);
		clearValue.Format	= m_TextureDesc.Format;
		//clearValue.Color[0] = 0.5f;
		//clearValue.Color[1] = 0.2f;
		//clearValue.Color[2] = 0.7f;
		//clearValue.Color[3] = 1.0f;

		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = Format;
		desc.Width = static_cast<uint64>(Width);
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		const auto& heapProperties = D3D::HeapPropertiesDefault();
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			nullptr,
			IID_PPV_ARGS(&m_Resource)));

		m_UsageFlag = ResourceUsageFlag::Texture2D;

		m_Device->CreateShaderResourceView(this, ShaderResourceHandle);
		m_Device->CreateRenderTargetView(this, RenderTargetHandle);

		if (!Name.empty())
		{
			DebugName = Name;
			SetDebugName(DebugName);
		}
	}

	void D3D12RenderTexture::Resize(uint32 Width, uint32 Height)
	{
		m_TextureDesc.Width = Width;
		m_TextureDesc.Height = Height;

		Create(m_Device, Width, Height, m_TextureDesc.Format, DebugName);
	}

} // namespace Luden
