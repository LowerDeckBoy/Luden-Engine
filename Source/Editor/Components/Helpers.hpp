#pragma once

#include <Core/String.hpp>

namespace Luden
{
	inline void TableNextRowBegin(std::string_view Text)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", Text.data());
	}
} // namespace Luden

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
