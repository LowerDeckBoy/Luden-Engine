#pragma once

#include <Core/Types.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <D3D12AgilitySDK/d3d12shader.h>

namespace Luden
{
	enum class ShaderStageFlag
	{
		None,
		Vertex,
		Pixel,
		Amplification,
		Mesh,
		RayGen,
		ClosestHit,
		Miss
	};

	class D3D12Shader
	{
	public:
		D3D12Shader() = default;
		D3D12Shader(void* pData, usize Size, ShaderStageFlag ShaderStage)
			:
			Data(pData),
			Size(Size),
			ShaderStage(ShaderStage)
		{

		}
		~D3D12Shader()
		{
			if (Data)
			{
				Data = nullptr;
				Size = 0;
			}
		}

		D3D12_SHADER_BYTECODE Bytecode()
		{
			return { Data, Size };
		}

		const char* ShaderName = "";

		void* Data = nullptr;
		usize Size = 0;

		ShaderStageFlag ShaderStage{};

		bool IsValid() const
		{
			return Data != nullptr && Size != 0;
		}

		// Flag as true, if this Shader has specified RootSignature.
		// Thus, RootSignature object can be created from Shader Bytecode.
		bool bHasRootSignature = false;

	private:

	};
} // namespace Luden
