#pragma once

#include <ImGui/imgui_internal.h>

// Check if Win32 macro is defined.
#ifdef RGB
#undef RGB
#endif //RGB

namespace Luden::gui::Color
{
	constexpr ImVec4 RGB(float R, float G, float B)
	{
		return ImVec4(R / 255.0f, G / 255.0f, B / 255.0f, 1.0f);
	}

	constexpr ImVec4 RGBA(float R, float G, float B, float A = 1.0f)
	{
		return ImVec4(R / 255.0f, G / 255.0f, B / 255.0f, A);
	}

	constexpr ImVec4 BackgroundDark = RGBA(32, 32, 32);
	constexpr ImVec4 BackgroundDarkActive = BackgroundDark;

	constexpr ImVec4 HeaderDark		= ImVec4(0.097f, 0.097f, 0.097f, 1.0f);

	constexpr ImVec4 BackgroundLight = ImVec4(1.0f, 0.921f, 0.87f, 1.0f);

	constexpr ImVec4 BorderShadow	= ImVec4(1.0f, 0.659f, 0.231f, 1.0f);

	constexpr ImVec4 White			= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	constexpr ImVec4 Black			= ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	//constexpr ImVec4 Gray			= ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
	constexpr ImVec4 Gray			= RGBA(105.0f, 109.0f, 125.0f, 1.0f);

	constexpr ImVec4 Red			= ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr ImVec4 Green			= ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	constexpr ImVec4 Blue			= ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

	constexpr ImVec4 Coral			= ImVec4(1.0f, 0.329f, 0.431f, 1.0f);
	constexpr ImVec4 CoralHover		= ImVec4(1.0f, 0.0f, 0.3f, 1.0f);
	constexpr ImVec4 CoralActive	= ImVec4(1.0f, 0.0f, 0.5f, 1.0f);

} // namespace Luden::gui::Color
