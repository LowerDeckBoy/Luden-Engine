#include "Components.hpp"
#include <ImGui/imgui_internal.h>

namespace Luden::gui::Math
{
	void Float3(std::string_view Label, DirectX::XMFLOAT3& Float3)
	{
		if (ImGui::BeginTable(Label.data(), 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			ImGui::PushID(Label.data());
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text(Label.data());
			ImGui::TableNextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 1.25f);

			ImGui::Button("X");
			ImGui::SameLine();
			ImGui::DragFloat("##X", &Float3.x);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Button("Y");
			ImGui::SameLine();
			ImGui::DragFloat("##Y", &Float3.y);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::Button("Z");
			ImGui::SameLine();
			ImGui::DragFloat("##Z", &Float3.z);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PopStyleVar();
			ImGui::PopID();

			ImGui::EndTable();
		}
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