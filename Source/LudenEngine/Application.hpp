#pragma once

#include <Platform/Window.hpp>

#include <Engine/D3D12/D3D12RHI.hpp>

namespace Luden
{
	class Application
	{
	public:
		Application();
		~Application();

		void Initialize();
		void Run();
		void Shutdown();

	protected:
		Platform::Window Window;
		
		static LRESULT EditorWindowProc(::HWND Handle, UINT32 Message, WPARAM wParam, LPARAM lParam);

		D3D12RHI* m_D3D12RHI;

	};
} // namespace Luden
