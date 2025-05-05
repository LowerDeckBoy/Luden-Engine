#include <Engine/Scene/Scene.hpp>
#include "SceneHierarchyPanel.hpp"
#include <Core/Logger.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <ImGui/imgui.h>

namespace Luden::Panel
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::SetActiveScene(Scene* pScene)
	{
		m_ActiveScene = pScene;
	}

	void SceneHierarchyPanel::DrawPanel()
	{
		if (ImGui::TreeNodeEx("Actors", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (m_ActiveScene->Models.empty())
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text(ICON_FA_GHOST ICON_FA_GHOST ICON_FA_GHOST" Empty here... " ICON_FA_GHOST ICON_FA_GHOST ICON_FA_GHOST);
			
				ImGui::TreePop();
			
				return;
			}

			// TODO:
			//const auto& view = m_ActiveScene->GetRegistry()->view<ecs::NameComponent>();
			//for (auto [entity, name] : view.each())
			//{
			//	Entity entity(entity);
			//}

			for (auto& model : m_ActiveScene->Models)
			{
				Entity entity(*model);

				const auto& nameComponent = model->GetComponent<ecs::NameComponent>();

				ImGuiTreeNodeFlags flags =
					((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
					ImGuiTreeNodeFlags_Leaf;

				ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, nameComponent.Name.c_str());
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_SelectedEntity = entity;
				}

				// TODO
				//if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				//{
				//
				//}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	
} // namespace Luden::Panel
