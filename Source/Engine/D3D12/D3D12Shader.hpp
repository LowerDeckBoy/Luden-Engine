#pragma once

#include <Core/Types.hpp>
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

		D3D12_SHADER_BYTECODE Bytecode()
		{
			return { Data, Size };
		}

		void* Data = nullptr;
		usize Size = 0;

		ShaderStageFlag ShaderStage{};

		bool IsValid() const
		{
			return Data != nullptr && Size != 0;
		}

	private:

	};
} // namespace Luden
