#include <Engine/Scene/Scene.hpp>

#include "../Components/Components.hpp"
#include "../Components/Tooltip.hpp"
#include "SceneHierarchyPanel.hpp"
#include <Engine/Config.hpp>
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
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::TreeNodeEx("Config", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
			{
				ImGui::SeparatorText("Config");
				auto& config = Config::Get();

				if (ImGui::BeginTable("##data", 2))
				{
					// Row 0
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("V-Sync:");
					static const char* syncing[]{ "Off", "On", "Half", "Third", "Quarter" };
					ImGui::TableNextColumn();
					ImGui::Combo("##Interval:", &config.SyncInterval, syncing, IM_ARRAYSIZE(syncing));

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					if (config.SyncInterval == 0)
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Set fixed frame rate:");
						gui::OnItemHover("Allows to limit frame rate to value in range [24;240].");
						ImGui::TableNextColumn();
						ImGui::Checkbox("##Limit frames", &config.bAllowFixedFrameRate);

						//ImGui::TableNextRow();
						//ImGui::TableNextColumn();
						//if (config.bAllowFixedFrameRate)
						//{
						//	ImGui::AlignTextToFramePadding();
						//	ImGui::Text("Frame rate:");
						//	ImGui::TableNextColumn();
						//	ImGui::SliderInt("##Frame rate:", &m_Timer->FrameLimit, 24, 240);
						//}
					}

					// Row 1;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Mesh shading: ");
					gui::OnItemHover("Whether to use mesh shading pipeline instead of vertex shading.");
					ImGui::TableNextColumn();
					ImGui::Checkbox("##Mesh shading", &config.bMeshShading);

					if (config.bMeshShading)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						ImGui::AlignTextToFramePadding();
						ImGui::Text("Meshlets: ");
						gui::OnItemHover("Check to draw debug meshlet instances.");
						ImGui::TableNextColumn();
						ImGui::Checkbox("##meshlets", &config.bDrawMeshlets);
					}

					// Temporarly

					// Row 2;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Raytracing: ");
					gui::OnItemHover("Check to dispatch ray tracing.");
					ImGui::TableNextColumn();
					ImGui::Checkbox("##raytracing", &config.bRaytracing);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Alpha mask: ");
					gui::OnItemHover("Check to enable alpha mask cutoff in pixel shaders.");
					ImGui::TableNextColumn();
					ImGui::Checkbox("##bAlphaMask", &config.bAlphaMask);

					ImGui::BeginDisabled();
					// Row 3;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Draw sky: ");
					ImGui::TableNextColumn();
					ImGui::Checkbox("##drawSky", &config.bDrawSky);

					// Row 4;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Draw grid: ");
					ImGui::TableNextColumn();
					ImGui::Checkbox("##drawGrid", &config.bDrawGrid);

					ImGui::EndDisabled();

					ImGui::EndTable();
				}

				ImGui::TreePop();
			}
			

			/*
			// Set output image.
			{
				ImGui::Text("Image to display:");

				const char* items[] = { "Scene", "BaseColor", "Normal", "Metallic-Roughness", "Emissive", "LightPass" };

				if (ImGui::Combo("##comb", &DisplayImageIndex, items, IM_ARRAYSIZE(items)))
				{
					switch (DisplayImageIndex)
					{
					case 0:
						SetSceneImage(m_Renderer->SceneTextures.Scene.ShaderResourceHandle);
						break;
					case 1:
						SetSceneImage(m_Renderer->GBuffer->BaseColor.ShaderResourceHandle);
						break;
					case 2:
						SetSceneImage(m_Renderer->GBuffer->Normal.ShaderResourceHandle);
						break;
					case 3:
						SetSceneImage(m_Renderer->GBuffer->MetallicRoughness.ShaderResourceHandle);
						break;
					case 4:
						SetSceneImage(m_Renderer->GBuffer->Emissive.ShaderResourceHandle);
						break;
					case 5:
						SetSceneImage(m_Renderer->LightingPass->RenderTexture.ShaderResourceHandle);
						break;
					}

				}
			}
			*/
			/*
			ImGui::SeparatorText(ICON_FA_VIDEO" Camera");
			if (ImGui::BeginTable("##cameraPanel", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
			{
				ImGui::TableSetupColumn("Property",  ImGuiTableColumnFlags_WidthStretch, 100.0f);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Position");
				ImGui::TableNextColumn();
				if (gui::Math::DrawFloat3("Position", m_Renderer->Camera->Position))
				{
					m_Renderer->Camera->Update();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Speed");
				gui::OnItemHover("Speed is controlable when mouse scroll is used when RBM is hold.");
		
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Speed", &m_Renderer->Camera->CameraSpeed, 1.0f, 1.0f, 250.0f);
				
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Field of View");
				ImGui::TableNextColumn();
				if (ImGui::DragFloat("##fov", &m_Renderer->Camera->FieldOfView, 1.0f, 1.0f, 90.0f))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Near Z");
				ImGui::TableNextColumn();
				if (ImGui::DragFloat("##zNear", &m_Renderer->Camera->zNear, 0.1f, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Far Z");
				ImGui::TableNextColumn();
				if (ImGui::DragFloat("##zFar", &m_Renderer->Camera->zFar, 1.0f, 1000.0f, 0.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::EndTable();
			}
			*/

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

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		
	}
	
} // namespace Luden::Panel
