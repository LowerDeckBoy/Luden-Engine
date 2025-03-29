#include "Components.hpp"
#include "../Colors.hpp"
#include <ImGui/imgui_internal.h>

namespace Luden::gui::Math
{
	bool Float3(std::string_view Label, DirectX::XMFLOAT3& Float3)
	{
		// Either when slider has been dragged or it's correnspoding button has been clicked.
		bool bActive = false;

		ImGui::PushID(Label.data());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 1.25f);

		ImGui::PushStyleColor(ImGuiCol_Button, Color::Red);
		if (ImGui::Button("##x")) 
		{ 
			Float3.x = 0.0f; 
			bActive = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &Float3.x)) 
		{ 
			bActive = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, Color::Green);
		if (ImGui::Button("##y")) 
		{ 
			Float3.y = 0.0f;
			bActive = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &Float3.y))
		{ 
			bActive = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, Color::Blue);
		if (ImGui::Button("##z"))
		{
			Float3.z = 0.0f;
			bActive = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &Float3.z))
		{
			bActive = true;
		}
		ImGui::PopItemWidth();

		ImGui::PopID();
		ImGui::PopStyleVar();

		return bActive;
	}

	void Float4(std::string_view Label, DirectX::XMFLOAT4& Float4)
	{
		if (ImGui::BeginTable(Label.data(), 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			ImGui::PushID(Label.data());
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(Label.data());
			ImGui::TableNextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() * 1.25f);

			ImGui::Button("X");
			ImGui::SameLine();
			ImGui::DragFloat("##X", &Float4.x);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Button("Y");
			ImGui::SameLine();
			ImGui::DragFloat("##Y", &Float4.y);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Button("Z");
			ImGui::SameLine();
			ImGui::DragFloat("##Z", &Float4.z);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Button("W");
			ImGui::SameLine();
			ImGui::DragFloat("##W", &Float4.w);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PopStyleVar();
			ImGui::PopID();

			ImGui::EndTable();

		}
	}

} // namespace Luden::gui::Math