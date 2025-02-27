#include <Core/String.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include "Types.hpp"

namespace Luden
{
	std::string_view ShaderModelToString(D3D_SHADER_MODEL ShaderModel)
	{
		switch (ShaderModel)
		{
		case D3D_SHADER_MODEL_6_0:	return "SM_6_0";
		case D3D_SHADER_MODEL_6_1:	return "SM_6_1";
		case D3D_SHADER_MODEL_6_2:	return "SM_6_2";
		case D3D_SHADER_MODEL_6_3:	return "SM_6_3";
		case D3D_SHADER_MODEL_6_4:	return "SM_6_4";
		case D3D_SHADER_MODEL_6_5:	return "SM_6_5";
		case D3D_SHADER_MODEL_6_6:	return "SM_6_6";
		case D3D_SHADER_MODEL_6_7:	return "SM_6_7";
		case D3D_SHADER_MODEL_6_8:	return "SM_6_8";

		default: return "Invalid";
		}

	}
} // namespace Luden
