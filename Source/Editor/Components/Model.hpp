#pragma once

#include <Engine/Graphics/Model.hpp>
#include <ImGui/imgui_internal.h>

namespace Luden::gui
{
	void DrawModelData(Model& InModel)
	{
        auto& tranformComponent = InModel.GetComponent<ecs::TransformComponent>();

        if (ImGui::BeginTable("##model", 2))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Position");
            ImGui::TableNextColumn();
            gui::Math::Float3("Position", tranformComponent.Translation);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Rotation");
            ImGui::TableNextColumn();
            gui::Math::Float3("Rotation", tranformComponent.Rotation);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Scale");
            ImGui::TableNextColumn();
            gui::Math::Float3("Scale", tranformComponent.Scale);

            ImGui::EndTable();
        }
    }

    void DrawModelData(Entity& ModelEntity)
    {
        auto& nameComponent     = ModelEntity.GetComponent<ecs::NameComponent>();
        auto& tranformComponent = ModelEntity.GetComponent<ecs::TransformComponent>();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name: %s", nameComponent.Name.c_str());

        if (ImGui::TreeNodeEx("Transforms", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen))
        {
            //if (ImGui::BeginTable("##model", 2))
            {
                ImGui::BeginTable("##model", 2);
                ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Position");
                ImGui::TableNextColumn();
                gui::Math::Float3("Position", tranformComponent.Translation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();
                gui::Math::Float3("Rotation", tranformComponent.Rotation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                gui::Math::Float3("Scale", tranformComponent.Scale);

                ImGui::EndTable();
            }

            ImGui::TreePop();
        }
       
    }

} // namespace Luden::gui
