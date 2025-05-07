#include "Application.hpp"
#include "Platform/Utility.hpp"
#include "Scene/SceneSerializer.hpp"
#include <Core/Assert.hpp>
#include <Core/Logger.hpp>

extern "C"
{
	__declspec(dllexport) extern const UINT  D3D12SDKVersion	= RHI_D3D12AGILITYSDK_VERSION;
	__declspec(dllexport) extern const char* D3D12SDKPath		= RHI_D3D12AGILITYSDK_PATH;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Luden
{
	AssetImporter Application::Importer;

	bool Application::bIsResizing = false;

	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Initialize()
	{
		Window.Create({
			.Title		= "Luden Engine",
			.Width		= 1920,
			.Height		= 1080,
			.WindowProc = &EditorWindowProc,
			.bMaximize	= false
		});

		m_D3D12RHI = new D3D12RHI(&Window);
		m_Renderer = new Renderer(&Window, m_D3D12RHI);

		Importer.Device = m_D3D12RHI->Device;

		MainScene = new Scene(&Importer);

		SceneSerializer::Load(&Importer, MainScene, "Scenes/scene_test.json");

		//m_Renderer->BuildScene(MainScene);
		MainScene->Build(m_D3D12RHI->Device);
		m_Renderer->ActiveScene = MainScene;

		m_Editor = std::make_unique<Editor>(&Window, m_Renderer, &m_Timer);
		m_Editor->SetActiveScene(MainScene);

		m_Renderer->Resize();

		bIsResizing = false;
		
	}

	void Application::Run()
	{
		m_Timer.Reset();

		while (!Window.bShouldClose)
		{
			Window.ProcessMessages();

			// Need to rework request handling.
			// Gets the job done, but is ugly.
			if (m_Renderer->bRequestCleanup)
			{
				m_Renderer->ReleaseActiveScene();
				m_Renderer->bRequestCleanup = false;
			}

			if (m_Renderer->bRequestSceneLoad)
			{
				m_Renderer->bRequestSceneLoad = false;
				if (MainScene)
				{
					m_Renderer->ReleaseActiveScene();
					MainScene = nullptr;
					MainScene = new Scene(&Importer);
				} 
				
				SceneSerializer::Load(&Importer, MainScene, m_Renderer->SceneToLoad);
				MainScene->Build(m_D3D12RHI->Device);
				//m_Renderer->BuildScene(MainScene);
				m_Renderer->ActiveScene = MainScene;
				m_Renderer->SceneToLoad = "";
				m_Editor->SetActiveScene(MainScene);
			}

			// If Window is minimized skip rendering.
			if (Window.IsMinimized())
			{
				continue;
			}

			if (bIsResizing)
			{
				m_Renderer->Resize();
				bIsResizing = false;
				LOG_DEBUG("Resized to {0}x{1}", Window.Width, Window.Height);
				
			}

			m_Timer.GetFrameTime();
			
			if (Config::Get().bAllowFixedFrameRate)
			{
				if (m_Timer.FrameTime <= (1000.0 / (f64)m_Timer.FrameLimit))
				{
					continue;
				}
			}

			m_Timer.Tick();

			m_Renderer->Update(m_Timer.DeltaTime);

			m_Renderer->BeginFrame();
			m_Editor->Begin();
			m_Renderer->Render(MainScene);
			m_Editor->End();
			m_Renderer->EndFrame();

			m_Renderer->Present(Config::Get().SyncInterval);

		}

		m_Renderer->GetRHI()->Wait();
		m_Renderer->GetRHI()->Flush();
	}

	void Application::Shutdown()
	{	
		m_Editor.reset();
		delete MainScene;
		delete m_Renderer;
		delete m_D3D12RHI;

		Window.Shutdown();
	}

	LRESULT Application::EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(Handle, Message, wParam, lParam))
		{
			return true;
		}

		switch (Message)
		{
		case WM_SIZE:
		{
			if (wParam == SIZE_MAXIMIZED)
			{
				bIsResizing = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				bIsResizing = true;
			}
			else
			{
				bIsResizing = false;
			}
			
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO mminfo = (LPMINMAXINFO)lParam;
		
			// Window min dimensions.
			mminfo->ptMinTrackSize.x = 1280;
			mminfo->ptMinTrackSize.y = 800;
		
			// Window max dimensions are the ones of current monitor display size.
			mminfo->ptMaxTrackSize.x = ::GetSystemMetrics(SM_CXMAXIMIZED);
			mminfo->ptMaxTrackSize.y = ::GetSystemMetrics(SM_CYMAXIMIZED);
		
			return 0;
		}
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				::PostQuitMessage(0);
			}
			break;
		}
		case WM_QUIT:
			FALLTHROUGH;
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			break;
		}
		}

		return ::DefWindowProcA(Handle, Message, wParam, lParam);
	}

} // namespace Luden
