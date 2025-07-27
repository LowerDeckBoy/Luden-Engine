#include "../Colors.hpp"
#include "Components.hpp"
#include <ImGui/imgui_stdlib.h>
#include <FontAwsome6/IconsFontAwesome6.h>

namespace Luden::gui
{
	bool Math::DrawFloat3(std::string_view Label, DirectX::XMFLOAT3& Float3)
	{
		// Either when slider has been dragged or it's correnspoding button has been clicked.
		bool bActive = false;

		ImGui::PushID(Label.data());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		//ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 1.25f);
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		//ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionAvail().x /3);// * 1.25f);

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

	void Math::DrawFloat4(std::string_view Label, DirectX::XMFLOAT4& Float4)
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

	void Math::EditColor3(std::string_view Label, DirectX::XMFLOAT3& Float3)
	{
		ImGui::Text(Label.data());
		ImGui::SameLine();
		ImGui::ColorEdit3("##editColor", (float*)&Float3);
	}

	void DrawTransformComponent(ecs::TransformComponent& Component)
	{
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##model", 2))
			{
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 60.0f);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Position");
				ImGui::TableNextColumn();
				if (Math::DrawFloat3("Position", Component.Translation))
				{
					Component.bDirty = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Rotation");
				ImGui::TableNextColumn();
				if (Math::DrawFloat3("Rotation", *(DirectX::XMFLOAT3*)&Component.Rotation))
				{
					Component.bDirty = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Scale");
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

	void DrawNameComponent(ecs::NameComponent& Component)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Name:");
		ImGui::SameLine();
		std::string last = Component.Name;
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
		if (ImGui::BeginTable("##pointLight", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 60.0f);
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
			Math::EditColor3("Ambient", Component.Ambient);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Radius");
			ImGui::TableNextColumn();
			ImGui::DragFloat("##radius", &Component.Radius, 1.0f, 1.0f, 0.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndTable();
		}
	}

	void DrawDirectionalLightComponent(ecs::DirectionalLightComponent& Component)
	{
		if (ImGui::BeginTable("##directionalLight", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 60.0f);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Position");
			ImGui::TableNextColumn();
			Math::DrawFloat3("Position", Component.Direction);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Ambient");
			ImGui::TableNextColumn();
			Math::EditColor3("Ambient", Component.Ambient);

			ImGui::EndTable();
		}
	}

	void AddOrRemoveComponent(Entity& /* Target */)
	{ 
		// Add/Remove components via editor
		if (ImGui::Button(ICON_FA_PLUS"Add"))
		{
			
		}

		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_MINUS"Remove"))
		{
			//Target.RemoveComponent<T>();
		}
	}

} // namespace Luden::gui::Math