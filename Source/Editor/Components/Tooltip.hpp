#pragma once

#include "../Colors.hpp"
#include <ImGui/imgui_internal.h>
#include <string>

namespace Luden::gui
{
	void OnItemHover(std::string_view TooltipText, bool bStationary = true)
	{
		if (ImGui::IsItemHovered((bStationary ? ImGuiHoveredFlags_Stationary : 0)))
		{
			ImGui::PushStyleColor(ImGuiCol_Border, Color::Gray);
			ImGui::SetTooltip(TooltipText.data());
			ImGui::PopStyleColor();
		}
	}
} //namespace Luden::gui
