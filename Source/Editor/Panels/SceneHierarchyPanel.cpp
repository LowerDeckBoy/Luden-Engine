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

			// Iterate over every entity that owns a NameComponent.
			const auto& view = m_ActiveScene->GetRegistry()->view<ecs::NameComponent>();
			for (auto [handle, name] : view.each())
			{
				Entity entity(m_ActiveScene->GetWorld(), handle);

				ImGuiTreeNodeFlags flags =
					((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
					ImGuiTreeNodeFlags_Leaf;

				ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, name.Name.data());
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_SelectedEntity = entity;
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
			{
				for (usize i = 0; i < m_ActiveScene->PointLights.size(); ++i)
				{
					
					//Entity entity(m_ActiveScene->GetWorld(), handle);
					Entity entity(m_ActiveScene->GetWorld(), m_ActiveScene->PointLights.at(i).GetHandle());

					ImGuiTreeNodeFlags flags =
						((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
						ImGuiTreeNodeFlags_Leaf;

					//ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, name.Name.data());
					ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, std::format("Point light {}", i).c_str());
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						m_SelectedEntity = entity;
					}
					ImGui::TreePop();
				}

				/*
				// Lights
				const auto& lightsView = m_ActiveScene->GetRegistry()->view<ecs::NameComponent, ecs::PointLightComponent>();
				for (auto [handle, name, light] : lightsView.each())
				{
					Entity entity(m_ActiveScene->GetWorld(), handle);

					ImGuiTreeNodeFlags flags =
						((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding |
						ImGuiTreeNodeFlags_Leaf;

					ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, name.Name.data());
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					{
						m_SelectedEntity = entity;
					}
					ImGui::TreePop();
				}
				*/
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	
} // namespace Luden::Panel
