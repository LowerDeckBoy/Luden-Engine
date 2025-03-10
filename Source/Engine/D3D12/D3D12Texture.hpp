#pragma once

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include <Core/File.hpp>

namespace Luden
{
	class D3D12RHI;

	enum class TextureUsageFlag
	{
		ShaderResource,
		UnorderedAccess,
		RenderTarget,
		DepthStencil,
	};

	struct TextureDesc
	{
		TextureUsageFlag	Usage;
		void*				Data;
		uint32				Width;
		uint32				Height;
		DXGI_FORMAT			Format;
		uint16				NumMips = 1;
		uint16				DepthOrArray = 1;
	};

	class D3D12Texture : public D3D12Resource
	{
	public:
		D3D12Texture() = default;
		// Create texture Resource from desc.
		D3D12Texture(D3D12Device* pDevice, TextureDesc Desc);
		// Create texture Resource from file.
		D3D12Texture(D3D12Device* pDevice, TextureDesc Desc, Filepath Path);
		~D3D12Texture();

		void Create(D3D12Device* pDevice, TextureDesc Desc);
		void Create(D3D12Device* pDevice, TextureDesc Desc, Filepath Path);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor RenderTargetHandle;
		D3D12Descriptor DepthStencilHandle;

	private:
		TextureDesc m_TextureDesc{};

	};

	class D3D12RenderTexture : public D3D12Resource
	{
	public:
		D3D12RenderTexture() = default;
		D3D12RenderTexture(D3D12Device* pDevice, TextureDesc Desc);
		~D3D12RenderTexture();

		void Create(D3D12Device* pDevice, TextureDesc Desc, std::string_view DebugName = "");

		void Resize(uint32 Width, uint32 Height);

		//void CreateShaderResource();
		//void CreateRenderTarget(D3D12DescriptorHeap* pDescriptorHeap);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor RenderTargetHandle;

	private:
		TextureDesc m_TextureDesc{};

		D3D12Device* m_Device;

	};
} // namespace Luden
