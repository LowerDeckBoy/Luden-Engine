#include "Editor.hpp"
#include "Theme.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Scene/SceneSerializer.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <Platform/FileDialog.hpp>
#include <Platform/Utility.hpp>

#include "Components/GUI.hpp"


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

			constexpr float iconsSize = 20 * 2.0f / 3.0f;
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
	   
		SetSceneImage(m_Renderer->GBuffer->BaseColor.ShaderResourceHandle);

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
		DrawEditorLayer();

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
		
		m_HierarchyPanel.SetActiveScene(pScene);

	}

	void Editor::SetSceneImage(D3D12Descriptor& TextureDescriptor)
	{
		if (TextureDescriptor.GpuHandle.ptr == 0)
		{
			return;
		}

		m_DrawImageAddress = (ImTextureID)TextureDescriptor.GpuHandle.ptr;

		//const auto& viewportSize = ImGui::GetContentRegionAvail();
		//ImGui::Image((ImTextureID)TextureDescriptor.GpuHandle.ptr, viewportSize);
	}

	void Editor::DrawEditorLayer()
	{
		DrawMainMenuBar();

		ImGui::Begin("Hierarchy");

		//DrawSceneControlPanel();
		m_HierarchyPanel.DrawPanel();
		DrawPropertyPanel();

		ImGui::End();

		DrawSceneImage();
	}

	void Editor::DrawMainMenuBar()
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			gui::SeparatorText("Scene");

			if (ImGui::MenuItem(ICON_FA_SD_CARD" Save"))
			{
				// TODO:
			}
			
			if (ImGui::MenuItem(ICON_FA_FILE_CODE" Load scene"))
			{	
				auto selected = Platform::FileDialog::Open(Platform::FOpenDialogOptions{
					.FilterExtensions = Platform::EExtensionFilter::Scene,
					.OpenLocation = "D:\\Dev\\Engines\\Luden\\Source\\Editor\\Scenes",
					.Title = "Select a scene file"
					});
					
				if (!selected.empty())
				{
					m_HierarchyPanel.ResetSelection();
					m_Renderer->bRequestSceneLoad = true;
					m_Renderer->SceneToLoad = selected;
				}
			}
			
			if (ImGui::MenuItem(ICON_FA_FOLDER_CLOSED" Unload scene"))
			{
				m_Renderer->bRequestCleanup = true;
				m_HierarchyPanel.ResetSelection();
			}

			if (ImGui::MenuItem(ICON_FA_CHESS_KNIGHT" Add model"))
			{
				//auto selected = Platform::FileDialog::Open("D:\\Dev\\Engines\\Luden\\Build\\Debug\\Assets\\");
				auto selected = Platform::FileDialog::Open(Platform::FOpenDialogOptions{
					.FilterExtensions = Platform::EExtensionFilter::Model,
					.OpenLocation = "D:\\Dev\\Engines\\Luden\\Assets\\Models\\",
					.Title = "Select a model file"
					});

				if (!selected.empty())
				{
					m_Renderer->ActiveScene->AddModel(selected);
				}
			}

			ImGui::Separator();
			if (ImGui::MenuItem(ICON_FA_POWER_OFF" Exit"))
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
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
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

			ImGui::TreePop();
		}
	}

	void Editor::DrawSceneImage() const
	{
		ImGui::Begin(ICON_FA_DESKTOP" Scene");
		const auto& viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(m_DrawImageAddress, viewportSize);
		if (ImGui::IsItemHovered())
		{
			m_Renderer->Camera->IsInViewport = true;
		}
		else
		{
			m_Renderer->Camera->IsInViewport = false;
		}

		ImGui::End();
	}

	void Editor::DrawPropertyPanel()
	{
		ImGui::Begin("Properties");

		if (m_HierarchyPanel.GetSelectedEntity().IsAlive())
		{
			m_PropertyPanel.DrawEntity(m_HierarchyPanel.GetSelectedEntity());
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

		ImGui::Text("VRAM: %dMB", m_Renderer->GetRHI()->Adapter->QueryAdapterMemory());

	}

	void Editor::DrawLightData()
	{
		if (ImGui::TreeNodeEx("Lighting", ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth))
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
