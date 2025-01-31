#pragma once

#include <ImGui/imgui_internal.h>

namespace Luden::gui
{
	inline void SeparatorVertical()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	}

	inline void SeparatorHorizontal()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
	}

} // namespace Luden::gui
