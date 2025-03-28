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
			D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&m_Resource)
		));

	}

	D3D12Texture::~D3D12Texture()
	{
	}

	D3D12RenderTexture::D3D12RenderTexture(D3D12Device* pDevice, TextureDesc Desc)
	{
		Create(pDevice, Desc);
	}

	D3D12RenderTexture::~D3D12RenderTexture()
	{
		Release();
	}

	void D3D12RenderTexture::Create(D3D12Device* pDevice, TextureDesc Desc, std::string_view DebugName)
	{
		if (IsValid())
		{
			Release();
		}

		m_Device = pDevice;
		m_TextureDesc = Desc;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = 0.5f;
		clearValue.Color[1] = 0.2f;
		clearValue.Color[2] = 0.7f;
		clearValue.Color[3] = 1.0f;
		clearValue.Format = Desc.Format;

		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format				= Desc.Format;
		desc.Width				= static_cast<uint64>(Desc.Width);
		desc.Height				= Desc.Height;
		desc.MipLevels			= Desc.NumMips;
		desc.DepthOrArraySize	= Desc.DepthOrArray;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Alignment			= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.SampleDesc			= { 1, 0 };
		desc.Flags				= D3D12_RESOURCE_FLAG_NONE;

		switch (Desc.Usage)
		{
		case TextureUsageFlag::UnorderedAccess:
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			break;
		case TextureUsageFlag::RenderTarget:
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			break;
		case TextureUsageFlag::DepthStencil:
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			break;
		}
		
		const auto& heapProperties = D3D::HeapPropertiesDefault();
		VERIFY_D3D12_RESULT(pDevice->LogicalDevice->CreateCommittedResource2(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			nullptr,
			IID_PPV_ARGS(&m_Resource)));

		if (!DebugName.empty())
		{
			NAME_D3D12_OBJECT(m_Resource.Get(), DebugName);
		}

		m_UsageFlag = ResourceUsageFlag::Texture2D;

		m_Device->CreateShaderResourceView(this, ShaderResourceHandle);
		m_Device->CreateRenderTargetView(this, RenderTargetHandle);

	}

	void D3D12RenderTexture::Resize(uint32 Width, uint32 Height)
	{
		m_TextureDesc.Width = Width;
		m_TextureDesc.Height = Height;

		Create(m_Device, m_TextureDesc);
	}

} // namespace Luden
