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
		DrawPostProcessConfig();
		DrawAmbientOcclusionConfig();

		ImGui::End();
	}

	void ConfigPanel::DrawDebugPanel()
	{
		ImGui::Begin(ICON_FA_BUG" Debug");

		if (ImGui::BeginTable("##timers", 2))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Present:");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->PresentRenderTime);
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("G-Buffer:");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->GBuffer->RenderTime);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("LightPass:");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->LightingPass->RenderTime);

			if (Config::Get().bEnableBloom)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Bloom:");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->BloomPass->RenderTime);
			}

			if (Config::Get().bEnableTonemapping)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Tonemapping:");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->TonemappingPass->RenderTime);
			}

			if (Config::Get().bEnableFXAA)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("FXAA:");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->FXAAPass->RenderTime);
			}

			ImGui::EndTable();
		}


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
				ImGui::SetNextItemWidth(-1.0f);
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

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable Post-Process: ");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Post-Process", &config.bEnablePostProcess);

				ImGui::EndTable();
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
				ImGui::SetNextItemWidth(-1.0f);
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
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::DragFloat("##Speed", &m_Renderer->Camera->CameraSpeed, 1.0f, 1.0f, 250.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Field of View");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::DragFloat("##fov", &m_Renderer->Camera->FieldOfView, 1.0f, 1.0f, 90.0f))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Near Z");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::DragFloat("##zNear", &m_Renderer->Camera->zNear, 0.1f, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Far Z");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::DragFloat("##zFar", &m_Renderer->Camera->zFar, 1.0f, 1000.0f, 0.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					m_Renderer->Camera->Resize();
				}

				ImGui::EndTable();
			}
		}
	}

	void ConfigPanel::DrawPostProcessConfig()
	{
		if (ImGui::CollapsingHeader("Post-Process"))
		{
			DrawBloomConfig();
			DrawTonemappingConfig();
			DrawAntiAliasingConfig();
		}
	}

	void ConfigPanel::DrawBloomConfig()
	{
		if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_FramePadding))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedFit))
			{
				
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Enable", &Config::Get().bEnableBloom);

				if (!Config::Get().bEnableBloom)
				{
					ImGui::BeginDisabled();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Threshold");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold:", &m_Renderer->BloomPass->Parameters.Threshold, 0.0f, 10.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Intensity");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Intensity:", &m_Renderer->BloomPass->Parameters.Intensity, 0.0f, 50.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Exposure");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Exposure:", &m_Renderer->BloomPass->Parameters.Exposure, 0.0f, 5.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Gamma");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Gamma:", &m_Renderer->BloomPass->Parameters.Gamma, 0.0f, 10.0f);

				if (!Config::Get().bEnableBloom)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawAmbientOcclusionConfig()
	{
		if (ImGui::CollapsingHeader("SSAO"))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Enable", &Config::Get().bEnableSSAO);

				ImGui::TableNextRow();
				ImGui::TableNextColumn(); ImGui::Text("");
				if (!Config::Get().bEnableSSAO)
				{
					ImGui::BeginDisabled();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Compute - test");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Compute", &Config::Get().bSSAOCompute);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Radius");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Radius", &m_Renderer->SSAOPass->Parameters.Radius, 0.0f, 10.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Bias");
				ImGui::TableNextColumn();
				ImGui::SliderFloat("##Bias", &m_Renderer->SSAOPass->Parameters.Bias, 0.0f, 10.0f);

				if (!Config::Get().bEnableSSAO)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}
		}
	}

	void ConfigPanel::DrawAntiAliasingConfig()
	{
		if (ImGui::TreeNodeEx("Anti-Aliasing", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable FXAA");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Enable:", &Config::Get().bEnableFXAA);

				if (!Config::Get().bEnableFXAA)
				{
					ImGui::BeginDisabled();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Subpixel quality");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1);
				ImGui::SliderFloat("##Subpixel quality:", &m_Renderer->FXAAPass->Parameters.QualitySubpixel, 0.0f, 8.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Threshold");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold:", &m_Renderer->FXAAPass->Parameters.EdgeThreshold, 0.1f, 5.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Threshold Min");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold Min:", &m_Renderer->FXAAPass->Parameters.EdgeThresholdMin, 0.1f, 5.0f);

				if (!Config::Get().bEnableFXAA)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawTonemappingConfig()
	{
		if (ImGui::TreeNodeEx("Tonemapping", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableTonemapping);

				if (!Config::Get().bEnableTonemapping)
				{
					ImGui::BeginDisabled();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Exposure");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::DragFloat("##Exposure", &m_Renderer->TonemappingPass->Exposure);

				static const char* types[5] = { "None", "Reinhard", "Gamma Correction", "Uncharted2", "ACES" };
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Type");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Combo("##Type", &m_Renderer->TonemappingPass->Type, types, IM_ARRAYSIZE(types));

				if (!Config::Get().bEnableTonemapping)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

} // namespace Luden::Panel
