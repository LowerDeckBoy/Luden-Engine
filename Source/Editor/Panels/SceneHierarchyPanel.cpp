#include <Engine/Scene/Scene.hpp>
#include "../Components/Components.hpp"
#include "../Components/Tooltip.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "SceneHierarchyPanel.hpp"
#include <Engine/Config.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <ImGui/imgui.h>
#include <Engine/ECS/Components/LightComponent.hpp>

namespace Luden::Panel
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{

	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::SetActiveScene(Scene* pScene, Renderer* pRenderer)
	{
		m_ActiveScene	= pScene;
		m_Renderer		= pRenderer;

		DisplayImageAddress = m_Renderer->GBuffer->BaseColor.ShaderResourceHandle.GpuHandle.ptr;
	}

	void SceneHierarchyPanel::DrawPanel()
	{
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (m_ActiveScene->Models.empty())
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(ICON_FA_GHOST ICON_FA_GHOST ICON_FA_GHOST" Empty here... " ICON_FA_GHOST ICON_FA_GHOST ICON_FA_GHOST);

				ImGui::TreePop();

				return;
			}

			if (ImGui::BeginTable("hierachy", 2, ImGuiTableFlags_SizingStretchProp))
			{
				// Iterate over every entity that owns a NameComponent.
				const auto& view = m_ActiveScene->GetRegistry()->view<ecs::NameComponent>();

				for (auto [handle, name] : view.each())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					const char* icon = name.bVisibleInScene ? ICON_FA_EYE : ICON_FA_EYE_SLASH;

					ImGui::PushID(name.Name.data());
					ImGui::PushStyleColor(ImGuiCol_Button, gui::Color::BackgroundDark);
					if (ImGui::Button(icon))
					{
						name.bVisibleInScene = !name.bVisibleInScene;
					}
					ImGui::PopStyleColor();
					ImGui::PopID();

					ImGui::TableNextColumn();

					Entity entity(m_ActiveScene->GetWorld(), handle);

					ImGuiTreeNodeFlags flags =
						((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
						ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;
					ImGui::SetNextItemWidth(-1.0f);

					ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, name.Name.data());
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						m_SelectedEntity = entity;
					}

					ImGui::TreePop();
				}

				ImGui::EndTable();
				ImGui::TreePop();
			}
		}
	}
	
} // namespace Luden::Panel
