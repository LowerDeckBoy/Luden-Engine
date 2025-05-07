#pragma once

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Resource.hpp"
#include <Core/File.hpp>
#include <array>

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
		void*				Data = nullptr;
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
		//void CreateFromMemory(D3D12Device* pDevice, TextureDesc Desc);
		//void CreateFromFile(D3D12Device* pDevice, TextureDesc Desc, Filepath Path);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor RenderTargetHandle;
		D3D12Descriptor DepthStencilHandle;

		TextureDesc& GetTextureDesc()
		{
			return m_TextureDesc;
		}

		Filepath& GetFilepath()
		{
			return m_Filepath;
		}

		void SetFilepath(Filepath Path)
		{
			m_Filepath = Path;
		}
		
	private:
		TextureDesc m_TextureDesc{};
		Filepath m_Filepath;

	}; // class D3D12Texture

	class D3D12RenderTexture : public D3D12Resource
	{
	public:
		D3D12RenderTexture() = default;
		D3D12RenderTexture(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, std::string_view Name = "");
		D3D12RenderTexture(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, std::array<float, 4> ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f }, std::string_view Name = "");
		~D3D12RenderTexture();

		void Create(D3D12Device* pDevice, uint32 Width, uint32 Height, DXGI_FORMAT Format, std::array<float, 4> ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f }, std::string_view Name = "");

		void Resize(uint32 Width, uint32 Height);

		D3D12Descriptor ShaderResourceHandle;
		D3D12Descriptor RenderTargetHandle;

		DXGI_FORMAT& GetFormat()
		{
			return m_TextureDesc.Format;
		}

	private:
		TextureDesc m_TextureDesc{};

		D3D12Device* m_Device;

		std::array<float, 4> m_ClearColor;

	}; // class D3D12RenderTexture
} // namespace Luden
