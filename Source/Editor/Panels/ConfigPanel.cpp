#include <Engine/Renderer/Renderer.hpp>
#include "../Components/Tooltip.hpp"
#include "../Components/Helpers.hpp"
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
		ImGui::Begin(ICON_FA_GEAR" Config");

		DrawSceneConfig();
		DrawSceneCameraConfig();
		DrawPostProcessConfig();
		

		ImGui::End();
	}

	void ConfigPanel::DrawDebugPanel()
	{
		ImGui::Begin(ICON_FA_BUG" Debug");

		if (ImGui::BeginTable("##timers", 2))
		{
			ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

			TableNextRowBegin("Present");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->PresentRenderTime);

			TableNextRowBegin("Update");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->UpdateRenderTime);

			TableNextRowBegin("G-Buffer");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->GBuffer->RenderTime);

			TableNextRowBegin("LightPass");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::Text("%.3f ms", m_Renderer->LightingPass->RenderTime);

			if (Config::Get().bEnableBloom)
			{
				TableNextRowBegin("Bloom");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->BloomPass->RenderTime);
			}

			if (Config::Get().bEnableTonemapping)
			{
				TableNextRowBegin("Tonemapping");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->TonemappingPass->RenderTime);
			}

			if (Config::Get().bEnableFXAA)
			{
				TableNextRowBegin("FXAA");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->FXAAPass->RenderTime);
			}

			if (Config::Get().bEnableFilmEffects)
			{
				TableNextRowBegin("FilmEffects");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->FilmEffectsPass->RenderTime);
			}

			if (Config::Get().bEnableSSR)
			{
				TableNextRowBegin("SSR");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->SSRPass->RenderTime);
			}

			if (Config::Get().bEnableSSAO)
			{
				TableNextRowBegin("SSAO");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Text("%.3f ms", m_Renderer->SSAOPass->RenderTime);
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

			if (ImGui::BeginTable("##data", 2, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				// Row 0
				TableNextRowBegin("V-Sync");
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
						ImGui::Text("Frame rate");
						ImGui::TableNextColumn();
						ImGui::SliderInt("##Frame rate", &m_Timer->FrameLimit, 24, 240);
					}
				}

				TableNextRowBegin("Draw meshlets");
				gui::OnItemHover("Check to draw debug meshlet instances.");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Draw Meshlets", &config.bDrawMeshlets);

				TableNextRowBegin("Meshlet GPU culling");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Meshlet culling", &config.bMeshletCulling);

				// Row 3;
				TableNextRowBegin("Raytracing");
				gui::OnItemHover("Check to dispatch ray tracing.");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##raytracing", &config.bRaytracing);

				TableNextRowBegin("Alpha masking");
				gui::OnItemHover("Check to enable alpha mask cutoff in pixel shaders.");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##bAlphaMask", &config.bAlphaMask);

				TableNextRowBegin("Draw Indirect");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##bDrawIndirect", &config.bDrawIndirect);

				TableNextRowBegin("Enable Post-Process");
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
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Position");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (gui::Math::DrawFloat3("Position", m_Renderer->Camera->Position))
				{
					m_Renderer->Camera->Update();
				}

				TableNextRowBegin("Speed");
				gui::OnItemHover("Speed is controlable when mouse scroll is used when RBM is hold.");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::DragFloat("##Speed", &m_Renderer->Camera->CameraSpeed, 1.0f, 1.0f, 250.0f);

				TableNextRowBegin("Field of View");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::DragFloat("##fov", &m_Renderer->Camera->FieldOfView, 1.0f, 1.0f, 90.0f))
				{
					m_Renderer->Camera->Resize();
				}

				TableNextRowBegin("Near Z");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::DragFloat("##zNear", &m_Renderer->Camera->zNear, 0.1f, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
				{
					m_Renderer->Camera->Resize();
				}
				TableNextRowBegin("Far Z");
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
			DrawAmbientOcclusionConfig();
			DrawSpaceScreenReflectionsConfig();
			DrawScatteringConfig();
			DrawFilmEffectsConfig();
			DrawAtmosphereConfig();
		}
	}

	void ConfigPanel::DrawBloomConfig()
	{
		if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Enable", &Config::Get().bEnableBloom);

				if (!Config::Get().bEnableBloom)
				{
					ImGui::BeginDisabled();
				}

				TableNextRowBegin("Threshold");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold:", &m_Renderer->BloomPass->Parameters.Threshold, 0.0f, 5.0f);

				TableNextRowBegin("Threshold Knee");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold Knee:", &m_Renderer->BloomPass->Parameters.ThresholdKnee, 0.0f, 3.0f);

				TableNextRowBegin("Intensity");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Intensity:", &m_Renderer->BloomPass->Parameters.Intensity, 0.0f, 50.0f, "%.1f");

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

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
		if (ImGui::TreeNodeEx("SSAO", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableSSAO);

				if (!Config::Get().bEnableSSAO)
				{
					ImGui::BeginDisabled();
				}

				TableNextRowBegin("Radius");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Radius", &m_Renderer->SSAOPass->Parameters.Radius, 0.5f, 10.0f, "%.1f");

				TableNextRowBegin("Power");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Power", &m_Renderer->SSAOPass->Parameters.Power, 0.1f, 10.0f, "%.1f");

				if (!Config::Get().bEnableSSAO)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawScatteringConfig()
	{
		if (ImGui::TreeNodeEx("Volumetric Scattering", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##paramaters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableScattering);

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawAtmosphereConfig()
	{
		if (ImGui::TreeNodeEx("Atmosphere Scattering", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##paramaters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableAtmosphere);

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawSpaceScreenReflectionsConfig()
	{
		if (ImGui::TreeNodeEx("Screen Space Reflections", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##paramaters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableSSR);

				TableNextRowBegin("Ray steps");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##steps", &m_Renderer->SSRPass->Parameters.RaySteps, 0.0f, 10.0f);

				TableNextRowBegin("Ray threshold");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##threshold", &m_Renderer->SSRPass->Parameters.RayThreshold, 0.0f, 10.0f);

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawFilmEffectsConfig()
	{
		if (ImGui::TreeNodeEx("Film Effects", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableFilmEffects);

				if (!Config::Get().bEnableFilmEffects)
				{
					ImGui::BeginDisabled();
				}

				TableNextRowBegin("Enable Chromatic Aberration");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable Chromatic Aberration", &Config::Get().bEnableChromaticAberration);

				TableNextRowBegin("Enable Film Grain");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable Film Grain", &Config::Get().bEnableFilmGrain);

				TableNextRowBegin("Enable Lens Distortion");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable Lens Distortion", &Config::Get().bEnableLensDistortion);

				TableNextRowBegin("Lens Distortion Intensity");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Lens Distortion Intensity", &m_Renderer->FilmEffectsPass->Parameters.LensDistortionIntensity, 0.0f, 1.0f);

				if (!Config::Get().bEnableFilmEffects)
				{
					ImGui::EndDisabled();
				}

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void ConfigPanel::DrawAntiAliasingConfig()
	{
		if (ImGui::TreeNodeEx("Anti-Aliasing", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable FXAA");
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Enable:", &Config::Get().bEnableFXAA);

				if (!Config::Get().bEnableFXAA)
				{
					ImGui::BeginDisabled();
				}

				TableNextRowBegin("Subpixel quality");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1);
				ImGui::SliderFloat("##Subpixel quality:", &m_Renderer->FXAAPass->Parameters.QualitySubpixel, 0.0f, 8.0f, "%1.0f", ImGuiSliderFlags_AlwaysClamp);

				TableNextRowBegin("Threshold");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold:", &m_Renderer->FXAAPass->Parameters.EdgeThreshold, 0.1f, 1.0f);

				TableNextRowBegin("Threshold Min");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderFloat("##Threshold Min:", &m_Renderer->FXAAPass->Parameters.EdgeThresholdMin, 0.1f, 1.0f);

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
			if (ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_SizingFixedSame))
			{
				ImGui::TableSetupColumn("##A", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##B", ImGuiTableColumnFlags_WidthStretch);

				TableNextRowBegin("Enable");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Checkbox("##Enable", &Config::Get().bEnableTonemapping);

				if (!Config::Get().bEnableTonemapping)
				{
					ImGui::BeginDisabled();
				}

				TableNextRowBegin("Exposure");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::DragFloat("##Exposure", &m_Renderer->TonemappingPass->Exposure);

				static const char* types[8] = { "None", "ACES", "AgX", "AgX Punchy", "Reinhard", "Gamma Correction", "Uncharted2", "Hable" };
				TableNextRowBegin("Mode");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::Combo("##Mode", &m_Renderer->TonemappingPass->Mode, types, IM_ARRAYSIZE(types));

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
