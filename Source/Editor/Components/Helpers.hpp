#pragma once

#include <Core/String.hpp>
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

	inline void SeparatorText(std::string_view Text)
	{
		ImGui::SeparatorText(Text.data());
	}

} // namespace Luden::gui
