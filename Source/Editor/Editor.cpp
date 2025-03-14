#include "Theme.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "Editor.hpp"
#include <Core/Logger.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <Platform/Utility.hpp>
#include "Components/Helpers.hpp"
#include "Components/Components.hpp"
#include "Components/Model.hpp"

namespace Luden
{
	Editor::Editor(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer)
	{
		Initialize(pParentWindow, pRenderer, pApplicationTimer);
	}

	Editor::~Editor()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();

		ImGui_ImplDX12_InvalidateDeviceObjects();
		ImGui::DestroyContext();
	}

	void Editor::Initialize(Platform::Window* pParentWindow, Renderer* pRenderer, Core::Timer* pApplicationTimer)
	{
		m_Renderer      = pRenderer;
		m_ParentWindow  = pParentWindow;
		m_Timer         = pApplicationTimer;
		
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		ImGuiIO& IO = ImGui::GetIO();
		m_Theme = &ImGui::GetStyle();

		gui::DarkTheme(*m_Theme);

		// Enable Docking
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
		IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

		ImGui_ImplWin32_EnableDpiAwareness();
		ImGui_ImplWin32_Init(pParentWindow->Handle);
		ImGui_ImplDX12_Init(m_Renderer->GetRHI()->Device->LogicalDevice,
			Config::Get().NumBackBuffers,
			m_Renderer->GetRHI()->SwapChain->GetSwapChainFormat(),
			m_Renderer->GetRHI()->Device->ShaderResourceHeap->GetHandleRaw(),
			m_Renderer->GetRHI()->Device->ShaderResourceHeap->GetHandleRaw()->GetCPUDescriptorHandleForHeapStart(),
			m_Renderer->GetRHI()->Device->ShaderResourceHeap->GetHandleRaw()->GetGPUDescriptorHandleForHeapStart());

		// Font and Icons
		{
			m_MainFont = IO.Fonts->AddFontFromFileTTF(FontPath, FontSize);

			constexpr float iconsSize = 18 * 2.0f / 3.0f;
			static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
			ImFontConfig iconsConfig;
			iconsConfig.MergeMode = true;
			iconsConfig.PixelSnapH = true;
			iconsConfig.GlyphMinAdvanceX = iconsSize;
			iconsConfig.SizePixels = iconsSize;
			IO.Fonts->AddFontFromFileTTF(IconsFontPath, iconsSize, &iconsConfig, iconsRanges);
		}

		m_MainViewport = ImGui::GetMainViewport();
		m_MainViewport->Flags |= ImGuiViewportFlags_TopMost;
		m_MainViewport->Flags |= ImGuiViewportFlags_OwnedByApp;


		if (!ImGui_ImplDX12_CreateDeviceObjects())
		{
			LOG_WARNING("Failed to call ImGui_ImplDX12_CreateDeviceObjects()");
		}
	   
	}

	void Editor::Begin()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();

		ImGui::NewFrame();

		ImGui::PushFont(m_MainFont);

		m_MainViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(m_MainViewport->ID, m_MainViewport);

	}

	void Editor::End()
	{
		DrawMainMenuBar();

		{
			ImGui::Begin("Hierarchy");
			
			DrawSceneControlPanel();
			DrawActorsData();
			DrawLightData();

			DrawPropertyPanel();
			
			ImGui::End();
		}
		
		{
			ImGui::Begin("Scene");

			SetSceneImage(m_Renderer->SceneTextures.Scene.ShaderResourceHandle);

			ImGui::End();
		}

		ImGui::PopFont();

		ImGui::EndFrame();
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_Renderer->GetRHI()->Frames.at(BackBufferIndex).GraphicsCommandList->GetHandleRaw());
	}

	void Editor::SetActiveScene(Scene* pScene)
	{
		m_CurrentScene = pScene;
	}

	void Editor::SetSceneImage(D3D12Descriptor& TextureDescriptor)
	{
		if (TextureDescriptor.GpuHandle.ptr == 0)
		{
			return;
		}

		const auto& viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)TextureDescriptor.GpuHandle.ptr, viewportSize);
	}

	void Editor::DrawMainMenuBar()
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				m_ParentWindow->bShouldClose = true;
			}

			ImGui::EndMenu();
		}

		DisplayDebugInfo();

		ImGui::EndMainMenuBar();
	}

	void Editor::DrawSceneControlPanel()
	{
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_FramePadding))
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
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Allows to limit frame rate to value in range [24;240].");
					}
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
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Mesh shading: ");
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Whether to use mesh shading pipeline instead of vertex shading.");
				}
				ImGui::TableNextColumn();
				ImGui::Checkbox("##Mesh shading", &config.bMeshShading);

				if (config.bMeshShading)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Meshlets: ");
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Check to draw debug meshlet instances.");
					}
					ImGui::TableNextColumn();
					ImGui::Checkbox("##meshlets", &config.bDrawMeshlets);
				}    

				// Temporarly

				// Row 2;
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Raytracing: ");
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Check to dispatch ray tracing.");
				}
				ImGui::TableNextColumn();
				ImGui::Checkbox("##raytracing", &config.bRaytracing);

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

			ImGui::SeparatorText("Camera");
			if (ImGui::BeginTable("##cameraPanel", 2))
			{
				ImGui::TableSetupColumn("Property",  ImGuiTableColumnFlags_WidthFixed, 60.0f);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Position");
				ImGui::TableNextColumn();
				if (gui::Math::Float3("Position", m_Renderer->Camera->Position))
				{
					m_Renderer->Camera->Update();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Speed");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Speed", &m_Renderer->Camera->CameraSpeed, 1.0f, 1.0f, 100.0f);
				
				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	void Editor::DrawPropertyPanel()
	{
		ImGui::Begin("Properties");

		if (m_SelectedEntity.IsAlive())
		{
			//auto& nameComponent = m_SelectedEntity.GetComponent<ecs::NameComponent>();
			//ImGui::Text("Name: %s", nameComponent.Name.c_str());
			gui::DrawModelData(m_SelectedEntity);
		}

		ImGui::End();
	}

	void Editor::DrawLogsPanel()
	{
		ImGui::Begin("Logs");

		ImGui::End();

	}

	void Editor::DisplayDebugInfo()
	{
		ImGui::SetNextItemWidth(500.0f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 375.0f);
		ImGui::Text("fps: %d %0.2f ms", m_Timer->FPS, m_Timer->Miliseconds);
	   
		gui::SeparatorVertical();

		ImGui::Text("Mem: %.3fMB", Platform::GetMemoryUsage());
		ImGui::Text("VRAM: %dMB", m_Renderer->GetRHI()->QueryAdapterMemory());

	}

	void Editor::DrawActorsData()
	{
		// Sanity check
		// For now, it's possible that no scene can be active at run-time.
		if (!m_CurrentScene)
		{
			return; 
		}

		if (ImGui::TreeNodeEx("Actors", ImGuiTreeNodeFlags_FramePadding))
		{
			if (m_CurrentScene->Models.empty())
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Empty scene...");
				ImGui::TreePop();

				return;
			}

			for (auto& model : m_CurrentScene->Models)
			{
				Entity entity(model);
				const auto& nameComponent = model.GetComponent<ecs::NameComponent>();
				
				ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

				ImGui::TreeNodeEx((void*)entity.GetHandle(), flags, nameComponent.Name.c_str());
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_SelectedEntity = (Entity)entity;

					//gui::DrawModelData(model);
				}
				
				//if (ImGui::TreeNodeEx(nameComponent.Name.c_str(), flags))
				//{
				//	gui::DrawModelData(model);
				//	ImGui::TreePop();
				//}
			}

			ImGui::TreePop();
		}

	}

	void Editor::DrawLightData()
	{
		if (ImGui::TreeNodeEx("Lighting", ImGuiTreeNodeFlags_FramePadding))
		{
			// Draw all lights here

			ImGui::TreePop();
		}
	}

	void Editor::DrawEntityComponents(Entity& Entity)
	{
		// Sanity check.
		if (!Entity.HasComponent<ecs::NameComponent>())
		{
			return;
		}

		auto& nameComponent = Entity.GetComponent<ecs::NameComponent>();

		ImGui::Text("Name: %s", nameComponent.Name.c_str());

	}

} // namespace Luden
