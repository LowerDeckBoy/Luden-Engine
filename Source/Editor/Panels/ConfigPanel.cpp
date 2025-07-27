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

		DrawSceneConfig();
		DrawSceneCameraConfig();
		DrawBloomConfig();

		ImGui::End();
	}

	void ConfigPanel::DrawSceneConfig()
	{
		if (ImGui::CollapsingHeader("Config"))
		{
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

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("LightPass Compute: ");
				gui::OnItemHover("Check to switch between dispatching LightPass as Compute or Pixel\nCurrently for testing only.");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##LightPass Compute", &config.bLightPassCompute);

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
				ImGui::TableNextColumn();
				ImGui::Checkbox("##bDrawIndirect", &config.bDrawIndirect);

				ImGui::EndTable();
			}

			// Set output image.
			{
				ImGui::Text("Image to display:");

				static const char* items[] = { "Scene", "BaseColor", "Normal", "Metallic-Roughness", "Emissive", "LightPass", "Raytracing", "Bloom - Test" };

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
					case 7:
						DisplayImageAddress = m_Renderer->BloomPass->RenderTarget.ShaderResourceHandle.GpuHandle.ptr;
						break;
					}
				}
			}
		}
	}

	void ConfigPanel::DrawSceneCameraConfig()
	{
		if (ImGui::CollapsingHeader("Camera"))
		{
			if (ImGui::BeginTable("##cameraPanel", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame))
			{
				//ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch, 100.0f);
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
	}

	void ConfigPanel::DrawBloomConfig()
	{
		if (ImGui::CollapsingHeader("Bloom"))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame))
			{
				//ImGui::Checkbox("Enable");

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Threshold");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Threshold:", &m_Renderer->BloomPass->Parameters.Threshold, 0.0f, 10.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Intensity");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Intensity:", &m_Renderer->BloomPass->Parameters.Intensity, 0.0f, 50.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Exposure");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Exposure:", &m_Renderer->BloomPass->Parameters.Exposure, 0.0f, 5.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Gamma");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Gamma:", &m_Renderer->BloomPass->Parameters.Gamma, 0.0f, 10.0f);

				ImGui::EndTable();
			}
		}

	}

} // namespace Luden::Panel
