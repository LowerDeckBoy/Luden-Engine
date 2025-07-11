#include <Engine/Renderer/Renderer.hpp>
#include "../Components/Tooltip.hpp"
#include "ConfigPanel.hpp"
#include <FontAwsome6/IconsFontAwesome6.h>
#include "../Components/Components.hpp"
#include <Engine/Config.hpp>

namespace Luden::Panel
{
	void ConfigPanel::Initialize(Renderer* pRenderer, Core::Timer* pTimer)
	{
		m_Renderer = pRenderer;
		m_Timer = pTimer;
	}

	void ConfigPanel::DrawPanel()
	{
		ImGui::Begin("Config");

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

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (config.bAllowFixedFrameRate)
				{
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Frame rate:");
					ImGui::TableNextColumn();
					ImGui::SliderInt("##Frame rate:", &m_Timer->FrameLimit, 24, 240);
				}
			}

			// Row 1;
			//ImGui::TableNextRow();
			//ImGui::TableNextColumn();

			//ImGui::AlignTextToFramePadding();
			//ImGui::Text("Mesh shading: ");
			//gui::OnItemHover("Whether to use mesh shading pipeline instead of vertex shading.");
			//ImGui::TableNextColumn();
			//ImGui::Checkbox("##Mesh shading", &config.bMeshShading);

			//if (config.bMeshShading)
			//{
			//	
			//}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Draw Meshlets: ");
			gui::OnItemHover("Check to draw debug meshlet instances.");
			ImGui::TableNextColumn();
			ImGui::Checkbox("##Draw Meshlets", &config.bDrawMeshlets);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Meshlet culling: ");
			ImGui::TableNextColumn();
			ImGui::Checkbox("##Meshlet culling", &config.bMeshletCulling);

			// Row 3;
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

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Draw Indirect: ");
			//gui::OnItemHover("Check to enable alpha mask cutoff in pixel shaders.");
			ImGui::TableNextColumn();
			ImGui::Checkbox("##bDrawIndirect", &config.bDrawIndirect);

			ImGui::EndTable();
		}

		// Set output image.
		{
			ImGui::Text("Image to display:");

			const char* items[] = { "Scene", "BaseColor", "Normal", "Metallic-Roughness", "Emissive", "LightPass", "Raytracing" };

			if (ImGui::Combo("##comb", &DisplayImageIndex, items, IM_ARRAYSIZE(items)))
			{
				switch (DisplayImageIndex)
				{
				case 0:
					DisplayImageAddress = m_Renderer->SceneTextures.Scene.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 1:
					DisplayImageAddress = m_Renderer->GBuffer->BaseColor.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 2:
					DisplayImageAddress = m_Renderer->GBuffer->Normal.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 3:
					DisplayImageAddress = m_Renderer->GBuffer->MetallicRoughness.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 4:
					DisplayImageAddress = m_Renderer->GBuffer->Emissive.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 5:
					DisplayImageAddress = m_Renderer->LightingPass->RenderTexture.ShaderResourceHandle.GpuHandle.ptr;
					break;
				case 6:
					DisplayImageAddress = m_Renderer->RaytracingOutput->ShaderResourceHandle.GpuHandle.ptr;	
					break;
				}
			}

			ImGui::SeparatorText(ICON_FA_VIDEO" Camera");
			if (ImGui::BeginTable("##cameraPanel", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
				{
					ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch, 100.0f);
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
		}

		ImGui::End();
	}
} // namespace Luden::Panel
