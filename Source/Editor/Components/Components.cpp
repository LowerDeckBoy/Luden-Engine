#include "../Colors.hpp"
#include "Components.hpp"
#include "Helpers.hpp"
#include "Tooltip.hpp"
#include <Engine/ECS/Entity.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <ImGui/ImGuizmo.h>
#include <ImGui/imgui_stdlib.h>

namespace Luden::gui
{
	bool Math::DrawFloat3(std::string_view Label, DirectX::XMFLOAT3& Float3)
	{
		// Either when slider has been dragged or it's correnspoding button has been clicked.
		bool bActive = false;

		ImGui::PushID(Label.data());
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::PushStyleColor(ImGuiCol_Button, Color::Red);
		if (ImGui::Button("##x")) 
		{ 
			Float3.x = 0.0f; 
			bActive = true;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &Float3.x, 1.0f, 0.0f, 0.0f, "%.1f"))
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
		if (ImGui::DragFloat("##Y", &Float3.y, 1.0f, 0.0f, 0.0f, "%.1f"))
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
		if (ImGui::DragFloat("##Z", &Float3.z, 1.0f, 0.0f, 0.0f, "%.1f"))
		{
			bActive = true;
		}
		ImGui::PopItemWidth();

		ImGui::PopID();
		ImGui::PopStyleVar();

		return bActive;
	}
	
	void Math::EditColor3(std::string_view /* Label */, DirectX::XMFLOAT3& Float3)
	{
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::ColorEdit3("##editColor", (float*)&Float3);
	}

	void DrawTransformComponent(ecs::TransformComponent& Component)
	{
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##model", 2))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Position");
				ImGui::TableNextColumn();
				if (Math::DrawFloat3("Position", Component.Translation))
				{
					Component.bDirty = true;
				}

				TableNextRowBegin("Rotation");
				ImGui::TableNextColumn();
				if (Math::DrawFloat3("Rotation", *(DirectX::XMFLOAT3*)&Component.Rotation))
				{
					Component.bDirty = true;
				}

				TableNextRowBegin("Scale");
				ImGui::TableNextColumn();
				if (Math::DrawFloat3("Scale", Component.Scale))
				{
					Component.bDirty = true;
				}

				ImGui::EndTable();
			}	

			ImGui::TreePop();
		}
	}

	void DrawNameComponent( ecs::NameComponent& Component)
	{
		ImGui::Checkbox("##visible", &Component.bVisibleInScene);
		ImGui::SameLine();
		std::string last = Component.Name;
		ImGui::AlignTextToFramePadding();
		if (ImGui::InputText("##name", &Component.Name, ImGuiInputTextFlags_None))
		{
			if (Component.Name.empty())
			{
				Component.Name = last;
			}
		}
	}

	void DrawPointLightComponent(ecs::PointLightComponent& Component)
	{
		if (ImGui::BeginTable("##pointLight", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Position");
			ImGui::TableNextColumn();
			Math::DrawFloat3("Position", Component.Position);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Ambient");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1);
			Math::EditColor3("Ambient", Component.Ambient);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Radius");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##radius", &Component.Radius, 1.0f, 1.0f, 0.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndTable();
		}
	}

	void DrawSpotLightComponent(ecs::SpotLightComponent& Component)
	{
		if (ImGui::BeginTable("##spotLight", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

			TableNextRowBegin("Position");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1);
			Math::DrawFloat3("Position", Component.Position);

			TableNextRowBegin("Direction");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1);
			Math::DrawFloat3("Direction", Component.Direction);

			TableNextRowBegin("Ambient");
			ImGui::TableNextColumn();
			Math::EditColor3("Ambient", Component.Ambient);

			TableNextRowBegin("Inner Cutoff");
			ImGui::TableNextColumn();
			ImGui::DragFloat("##Inner Cutoff", &Component.InnerCutoff, 1.0f, 1.0f, 0.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			TableNextRowBegin("Outer Cutoff");
			ImGui::TableNextColumn();
			ImGui::DragFloat("##Outer Cutoff", &Component.OuterCutoff, 1.0f, 1.0f, 0.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			TableNextRowBegin("Intensity");
			ImGui::TableNextColumn();
			ImGui::DragFloat("##Intensity", &Component.Intensity, 1.0f, 1.0f, 0.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndTable();
		}
	}

	void DrawDirectionalLightComponent(ecs::DirectionalLightComponent& Component)
	{
		if (ImGui::BeginTable("##directionalLight", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

			TableNextRowBegin("Direction");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			Math::DrawFloat3("Direction", Component.Direction);

			TableNextRowBegin("Ambient");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			Math::EditColor3("Ambient", Component.Ambient);

			TableNextRowBegin("Intensity");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##Intensity", &Component.Intensity, 1.0f, 0.0f, 10.0f);

			ImGui::EndTable();
		}
	}

} // namespace Luden::gui::Math