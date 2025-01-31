#include "Theme.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "Editor.hpp"
#include <Core/Logger.hpp>
#include <FontAwsome6/IconsFontAwesome6.h>
#include <Platform/Utility.hpp>
#include "Components/Helpers.hpp"

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

        gui::DarkTheme(pParentWindow, *m_Theme);

        // Enable Docking
        IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
        IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
        IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
        IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplWin32_Init(pParentWindow->Handle);
        ImGui_ImplDX12_Init(m_Renderer->GetRHI()->Device->Device,
            Config::Get().NumBackBuffers,
            m_Renderer->GetRHI()->SwapChain->GetSwapChainFormat(),
            m_Renderer->GetRHI()->ShaderResourceHeap->GetHandleRaw(),
            m_Renderer->GetRHI()->ShaderResourceHeap->GetHandleRaw()->GetCPUDescriptorHandleForHeapStart(),
            m_Renderer->GetRHI()->ShaderResourceHeap->GetHandleRaw()->GetGPUDescriptorHandleForHeapStart());


        m_MainViewport = ImGui::GetMainViewport();
        m_MainViewport->Flags |= ImGuiViewportFlags_TopMost;
        m_MainViewport->Flags |= ImGuiViewportFlags_OwnedByApp;

        //m_Renderer->Resize();
        
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
       
    }

    void Editor::Begin()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX12_NewFrame();

        ImGui::NewFrame();

        ImGui::PushFont(m_MainFont);

        m_MainViewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(m_MainViewport->ID);

    }

    void Editor::End()
    {
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

        {
            ImGui::Begin("Hierarchy");

            DrawSceneControlPanel();
            
            gui::SeparatorHorizontal();

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

    void Editor::SetSceneImage(D3D12Descriptor& TextureDescriptor)
    {
        if (TextureDescriptor.GpuHandle.ptr == 0)
        {
            return;
        }

        auto viewportSize = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)TextureDescriptor.GpuHandle.ptr, viewportSize);
    }

    void Editor::DrawSceneControlPanel()
    {
        if (ImGui::TreeNodeEx("Scene"))
        {
            gui::SeparatorHorizontal();

            auto& config = Config::Get();

            if (ImGui::BeginTable("##data", 2))
            {
                // Row 0
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("Sync interval:");
                static const char* syncing[]{ "Off", "On", "Half", "Third", "Quarter" };
                ImGui::TableNextColumn();
                ImGui::Combo("##Interval:", &config.SyncInterval, syncing, IM_ARRAYSIZE(syncing));

                // Temporarly
                ImGui::BeginDisabled();

                // Row 1
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("Raytracing: ");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##raytracing", &config.bRaytracing);

                // Row 2;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("Draw sky: ");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##drawSky", &config.bDrawSky);

                // Row 3;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("Draw grid: ");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##drawGrid", &config.bDrawGrid);

                ImGui::EndDisabled();

                ImGui::EndTable();

            }

            ImGui::TreePop();
        }
    }

    void Editor::DisplayDebugInfo()
    {
        ImGui::SetNextItemWidth(500.0f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 375.0f);
        ImGui::Text("fps: %d %0.2f ms", m_Timer->FPS, m_Timer->Miliseconds);
       
        gui::SeparatorVertical();

        ImGui::Text("mem: %.2fMB", Platform::ReadRAM());
        ImGui::Text("VRAM: %dMB", m_Renderer->GetRHI()->QueryAdapterMemory());
    }

} // namespace Luden
