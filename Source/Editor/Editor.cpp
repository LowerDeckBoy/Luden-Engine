#include <Engine/Renderer/Renderer.hpp>
#include "Editor.hpp"
#include "Misc/Theme.hpp"
#include <FontAwsome6/IconsFontAwesome6.h>
#include <Platform/FileDialog.hpp>
#include <Platform/Utility.hpp>

#include "Components/GUI.hpp"

namespace Luden
{
	D3D12Texture* Editor::EditorDirectoryTexture	= nullptr;
	D3D12Texture* Editor::EditorFileTexture			= nullptr;
	D3D12Texture* Editor::EditorGLTFTexture			= nullptr;
	D3D12Texture* Editor::EditorGLBTexture			= nullptr;
	D3D12Texture* Editor::EditorOBJTexture			= nullptr;
	D3D12Texture* Editor::EditorPNGTexture			= nullptr;
	D3D12Texture* Editor::EditorJPGTexture			= nullptr;
	D3D12Texture* Editor::EditorJPEGTexture			= nullptr;
	D3D12Texture* Editor::EditorBINTexture			= nullptr;

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

		delete EditorDirectoryTexture;
		delete EditorFileTexture;
		delete EditorGLTFTexture;
		delete EditorGLBTexture;
		delete EditorOBJTexture;
		delete EditorPNGTexture;
		delete EditorJPGTexture;
		delete EditorJPEGTexture;
		delete EditorBINTexture;
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

		if (!ImGui_ImplDX12_CreateDeviceObjects())
		{
			LOG_WARNING("Failed to call ImGui_ImplDX12_CreateDeviceObjects()");
		}
		
		//SetSceneImage(m_Renderer->GBuffer->BaseColor.ShaderResourceHandle);
		SetSceneImage(m_Renderer->SceneTextures.Scene.ShaderResourceHandle);
		
		m_ConfigPanel.Initialize(m_Renderer, m_Timer);

		Importer.Device = pRenderer->GetRHI()->Device;
		CreateEditorResources();

		//m_EditorCommandList = new D3D12CommandList(pRenderer->GetRHI()->Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		//m_EditorCommandQueue = new D3D12CommandQueue(pRenderer->GetRHI()->Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	}

	void Editor::Begin()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();

		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
		//https://github.com/CedricGuillemet/ImGuizmo

		ImGui::PushFont(m_MainFont);

		m_MainViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(m_MainViewport->ID, m_MainViewport);

	}

	void Editor::End()
	{
		DrawEditorLayer();

		ImGui::PopFont();

		ImGui::Render();
		ImGui::EndFrame();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_Renderer->GetRHI()->Frames.at(BackBufferIndex).GraphicsCommandList->GetHandleRaw());
	}

	void Editor::Render()
	{
		//if (!m_EditorCommandList->IsOpen())
		//{
		//	m_EditorCommandList->Open();
		//}
		m_EditorCommandList->Open();
		m_EditorCommandList->SetDescriptorHeap(m_Renderer->GetRHI()->Device->ShaderResourceHeap);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();

		ImGui::NewFrame();

		ImGui::PushFont(m_MainFont);

		m_MainViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(m_MainViewport->ID, m_MainViewport);

		DrawEditorLayer();

		ImGui::PopFont();

		ImGui::EndFrame();
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_EditorCommandList->GetHandleRaw());
		m_EditorCommandQueue->Execute({ m_EditorCommandList });

	}

	void Editor::SetActiveScene(Scene* pScene)
	{
		m_CurrentScene = pScene;
		
		m_HierarchyPanel.SetActiveScene(pScene, m_Renderer);

	}

	void Editor::SetSceneImage(D3D12Descriptor& TextureDescriptor)
	{
		if (TextureDescriptor.GpuHandle.ptr == 0)
		{
			return;
		}

		m_ConfigPanel.DisplayImageAddress = (ImTextureID)TextureDescriptor.GpuHandle.ptr;

	}

	void Editor::DrawEditorLayer()
	{
		DrawMainMenuBar();

		ImGui::Begin("Hierarchy");
		
		m_ConfigPanel.DrawPanel();
		m_HierarchyPanel.DrawPanel();
		m_ContentBrowserPanel.DrawPanel();
		DrawPropertyPanel();

		ImGui::End();

		m_ConfigPanel.DrawDebugPanel();

		DrawSceneImage();
		DrawSceneDebugView();
	}

	void Editor::DrawMainMenuBar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		if (!ImGui::BeginMainMenuBar())
		{
			ImGui::EndMainMenuBar();
			ImGui::PopStyleVar();
			return;
		}

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
					.OpenLocation = "D:\\Dev\\Engines\\Luden\\Assets\\Scenes",
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

		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::BeginMenu(ICON_FA_IMAGE" Render Target"))
			{
				if (ImGui::MenuItem("Scene"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->SceneTextures.Scene.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("Base Color"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GBuffer->BaseColor.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("NormalTBN"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GBuffer->Normal.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("NormalVS"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GBuffer->NormalVS.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("Metallic-Roughness"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GBuffer->MetallicRoughness.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("Emissive"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GBuffer->Emissive.ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("Depth Buffer"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->GetRHI()->SceneDepthBuffer->ShaderResourceHandle.GpuHandle.ptr;
				}
				if (ImGui::MenuItem("LightPass"))
				{
					m_ConfigPanel.DisplayImageAddress = m_Renderer->LightingPass->RenderTexture.ShaderResourceHandle.GpuHandle.ptr;
				}

				if (Config::Get().bRaytracing)
				{
					if (ImGui::MenuItem("Raytracing"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->RaytracingOutput->ShaderResourceHandle.GpuHandle.ptr;
					}
				}
				
				if (Config::Get().bEnableBloom)
				{
					if (ImGui::MenuItem("Bloom"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->BloomPass->RenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}
				}
				
				if (Config::Get().bEnableSSAO)
				{
					if (ImGui::MenuItem("Screen Space Ambient Occlusion"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->SSAOPass->SSAORenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}

					if (ImGui::MenuItem("Screen Space Ambient Occlusion Blur"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->SSAOPass->BlurRenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}
				}

				if (Config::Get().bEnableSSR)
				{
					if (ImGui::MenuItem("Screen Space Reflections - Test"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->SSRPass->RenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}
				}
				
				if (Config::Get().bEnableSky)
				{
					if (ImGui::MenuItem("Sky"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->SkyboxPass->DebugRenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}

					if (ImGui::MenuItem("Procedural Sky"))
					{
						m_ConfigPanel.DisplayImageAddress = m_Renderer->ProceduralSkyPass->DebugRenderTarget.ShaderResourceHandle.GpuHandle.ptr;
					}
				}
				
				ImGui::EndMenu();
			}

			// Scene lights

			if (ImGui::MenuItem(ICON_FA_LIGHTBULB" Add Directional Light"))
			{
				m_Renderer->ActiveScene->AddDirectionalLight();
			}

			if (ImGui::MenuItem(ICON_FA_LIGHTBULB" Add Point Light"))
			{
				m_Renderer->ActiveScene->AddPointLight();
			}

			if (ImGui::MenuItem(ICON_FA_LIGHTBULB" Add Spot Light"))
			{
				m_Renderer->ActiveScene->AddSpotLight();
			}

			ImGui::EndMenu();
		}

		DisplayDebugInfo();

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();
	}

	void Editor::DrawSceneImage() const
	{
		ImGui::Begin(ICON_FA_DESKTOP" Scene", nullptr, ImGuiWindowFlags_NoScrollbar);

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsWindowHovered())
			{
				m_Renderer->Camera->IsInViewport = true;
			}
			else
			{
				m_Renderer->Camera->IsInViewport = false;
			}
		}

		const auto& viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(m_ConfigPanel.DisplayImageAddress, viewportSize);

		if (ImGui::BeginDragDropTarget())
		{
			auto payload = ImGui::AcceptDragDropPayload("PAYLOAD_ITEM", ImGuiDragDropFlags_None);
			if (payload)
			{
				const char* str = (const char*)payload->Data;
				m_CurrentScene->AddModel(Filepath(str));
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::End();	
	}

	void Editor::DrawSceneDebugView() const
	{
		ImGui::Begin(ICON_FA_DESKTOP" Debug View");

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsWindowHovered())
			{
				m_Renderer->Camera->IsInViewport = true;
			}
			else
			{
				m_Renderer->Camera->IsInViewport = false;
			}
		}
		
		const ImVec2 windowSize = ImGui::GetWindowSize();
		const ImVec2 subimageSize = { windowSize.x / 4.0f, windowSize.y / 4.0f };
		const uint32 tableColumns = 4;
		const auto tableFlags = ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_SizingStretchSame;

		if (ImGui::BeginTable("##debugViewUpper", tableColumns, tableFlags))
		{
			auto getPositionWithOffset = [&](uint32 Offset, float Height) {
				const float width = ImGui::GetColumnWidth();
				const float height = Height - (Height - 35);
				return ImVec2{ (15 + (width * Offset)), height };
				};

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->GBuffer->BaseColor.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			ImGui::SetCursorPos(getPositionWithOffset(0, subimageSize.y));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("BaseColor");
			//if (ImGui::IsItemHovered()) {}

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->GBuffer->NormalVS.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			ImGui::SetCursorPos(getPositionWithOffset(1, subimageSize.y));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Normal View Space");


			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->GBuffer->Normal.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			ImGui::SetCursorPos(getPositionWithOffset(2, subimageSize.y));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Normal TBN");

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->GBuffer->MetallicRoughness.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			ImGui::SetCursorPos(getPositionWithOffset(3, subimageSize.y));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Metallic-Roughness");

			ImGui::EndTable();
		}

		const auto& viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(m_ConfigPanel.DisplayImageAddress, viewportSize);

		if (ImGui::BeginTable("##debugViewLower", tableColumns, tableFlags))
		{
			const auto winPos = ImGui::GetWindowSize().y;

			auto getPositionWithOffset = [&](uint32 Offset, float Height) {
				const float width = ImGui::GetColumnWidth();
				const float height = (Height - 15);
				return ImVec2{ (15 + (width * Offset)), height };
				};

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->GBuffer->Emissive.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			float currentY = ImGui::GetCursorPosY();
			ImGui::SetCursorPos(getPositionWithOffset(0, currentY - subimageSize.y + 15));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Emissive");

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->SSAOPass->SSAORenderTarget.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			currentY = ImGui::GetCursorPosY();
			ImGui::SetCursorPos(getPositionWithOffset(1, currentY - subimageSize.y + 15));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Ambient Occlusion");

			ImGui::TableNextColumn();
			ImGui::Image(m_Renderer->BloomPass->RenderTarget.ShaderResourceHandle.GpuHandle.ptr, subimageSize);
			currentY = ImGui::GetCursorPosY();
			ImGui::SetCursorPos(getPositionWithOffset(2, currentY - subimageSize.y + 15));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Text("Bloom");

			ImGui::EndTable();
		}

		ImGui::End();
	}

	void Editor::DrawPropertyPanel()
	{
		ImGui::Begin("Properties");

		if (m_HierarchyPanel.GetSelectedEntity().IsValid())
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
		ImGui::Text("FPS: %d %0.2f ms", m_Timer->FPS, m_Timer->Miliseconds);
	   
		gui::SeparatorVertical();

		ImGui::Text("Mem: %.3fMB", Platform::GetMemoryUsage());

		ImGui::Text("VRAM: %dMB", m_Renderer->GetRHI()->Adapter->QueryAdapterMemory());

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

	void Editor::CreateEditorResources()
	{
		EditorDirectoryTexture	= Importer.LoadTexture("../../Assets/Textures/Editor/folder-1485.png");
		EditorDirectoryTexture->SetDebugName("[Editor] Directory Icon Texture");

		EditorFileTexture = Importer.LoadTexture("../../Assets/Textures/Editor/file-1453.png");
		EditorFileTexture->SetDebugName("[Editor] File Icon Texture");

		EditorGLTFTexture = Importer.LoadTexture("../../Assets/Textures/Editor/gltf-file-icon.png");
		EditorGLTFTexture->SetDebugName("[Editor] glTF Icon Texture");

		EditorGLBTexture = Importer.LoadTexture("../../Assets/Textures/Editor/glb-file-icon.png");
		EditorGLBTexture->SetDebugName("[Editor] glb Icon Texture");
		
		EditorOBJTexture = Importer.LoadTexture("../../Assets/Textures/Editor/obj-file-icon.png");
		EditorOBJTexture->SetDebugName("[Editor] obj Icon Texture");

		EditorPNGTexture = Importer.LoadTexture("../../Assets/Textures/Editor/png-file-icon.png");
		EditorPNGTexture->SetDebugName("[Editor] png Icon Texture");

		EditorJPGTexture = Importer.LoadTexture("../../Assets/Textures/Editor/jpg-file-icon.png");
		EditorJPGTexture->SetDebugName("[Editor] jpg Icon Texture");

		EditorJPEGTexture = Importer.LoadTexture("../../Assets/Textures/Editor/jpeg-file-icon.png");
		EditorJPEGTexture->SetDebugName("[Editor] jpeg Icon Texture");

		EditorBINTexture = Importer.LoadTexture("../../Assets/Textures/Editor/bin-file-icon.png");
		EditorBINTexture->SetDebugName("[Editor] bin Icon Texture");
	}

} // namespace Luden
