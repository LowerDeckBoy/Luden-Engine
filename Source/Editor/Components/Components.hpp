#pragma once

#include <Core/String.hpp>
#include <DirectXMath.h>

namespace Luden::gui::Math
{
	extern void Float3(std::string_view Label, DirectX::XMFLOAT3& Float3);

	extern void Float4(std::string_view Label, DirectX::XMFLOAT4& Float4);
		
} // namespace Luden::gui::Math
