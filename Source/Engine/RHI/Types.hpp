#pragma once

enum D3D_SHADER_MODEL;

namespace Luden
{
	enum VendorID
	{
		Nvidia,
		AMD,
		Intel,
	};

	enum class ShaderModelFlag
	{
		// Highest HLSL Shader Model below 6_0.
		Invalid = -1,

		SM6_0,
		SM6_1,
		SM6_2,
		SM6_3,
		SM6_4,
		SM6_5,
		SM6_6,
		SM6_7,
		SM6_8,
	};

	std::string_view ShaderModelToString(D3D_SHADER_MODEL ShaderModel);

} // namespace Luden
