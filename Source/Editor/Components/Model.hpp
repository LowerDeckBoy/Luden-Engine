#pragma once

#include <Engine/Graphics/Model.hpp>
//#include <ImGui/imgui_internal.h>

namespace Luden::gui
{
	void DrawModelData(Model& InModel)
	{
        auto& nameComponent = InModel.GetComponent<ecs::NameComponent>();
        auto& transformComponent = InModel.GetComponent<ecs::TransformComponent>();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::InputText("##name", nameComponent.Name.data(), sizeof(nameComponent.Name));

        // Put a table with child nodes / meshes here

        // TODO:
        // Add/Remove components via editor
        ImGui::Button(ICON_FA_PLUS"Add");
        ImGui::SameLine();
        ImGui::Button(ICON_FA_MINUS"Remove");

        //entt::view

        if (ImGui::TreeNodeEx("Hierarchy", ImGuiTreeNodeFlags_FramePadding))
        {
            ImGui::BeginTable("##hierarchy", 1);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
    
            ImGui::EndTable();

            ImGui::TreePop();
        }

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
                gui::Math::DrawFloat3("Position", transformComponent.Translation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();
                gui::Math::DrawFloat3("Rotation", *(DirectX::XMFLOAT3*)&transformComponent.Rotation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                gui::Math::DrawFloat3("Scale", transformComponent.Scale);

                ImGui::EndTable();
            }

            ImGui::TreePop();
        }
    }

    void DrawModelData(Entity& ModelEntity)
    {
        auto& nameComponent     = ModelEntity.GetComponent<ecs::NameComponent>();
        auto& tranformComponent = ModelEntity.GetComponent<ecs::TransformComponent>();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::InputText("##name", nameComponent.Name.data(), sizeof(nameComponent.Name));

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
                gui::Math::DrawFloat3("Position", tranformComponent.Translation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();
                gui::Math::DrawFloat3("Rotation", *(DirectX::XMFLOAT3*)&tranformComponent.Rotation);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                gui::Math::DrawFloat3("Scale", tranformComponent.Scale);

                ImGui::EndTable();
            }

           

            
            ImGui::TreePop();
        }
       
    }

} // namespace Luden::gui
