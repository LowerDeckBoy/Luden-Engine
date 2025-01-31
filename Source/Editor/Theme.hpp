#pragma once

#include <Platform/Window.hpp>
#include "Colors.hpp"
#include <dwmapi.h>

// NOT DEFINITIVE VERSION

namespace Luden::gui
{
	inline void DarkTheme(Platform::Window* pWindow, ImGuiStyle& InStyle)
	{
		COLORREF captionColor = RGB(32, 32, 32);
		DwmSetWindowAttribute(pWindow->Handle, DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));

		ImGui::StyleColorsDark();

		InStyle.WindowRounding		= 1.0f;
		InStyle.WindowBorderSize	= 0.0f;
		InStyle.FrameRounding		= 1.0f;

		InStyle.Colors[ImGuiCol_Text]				= Color::White;
		InStyle.Colors[ImGuiCol_WindowBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_MenuBarBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_DockingEmptyBg]		= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_Border]				= Color::BackgroundDarkActive;
		InStyle.Colors[ImGuiCol_BorderShadow]		= Color::BorderShadow;

		InStyle.Colors[ImGuiCol_TitleBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_TitleBgActive]		= Color::BackgroundDarkActive;
		InStyle.Colors[ImGuiCol_TitleBgCollapsed]	= Color::CoralHover;

		InStyle.Colors[ImGuiCol_Tab]				= Color::Gray;
		InStyle.Colors[ImGuiCol_TabActive]			= Color::CoralActive;
		InStyle.Colors[ImGuiCol_TabHovered]			= Color::CoralHover;
		InStyle.Colors[ImGuiCol_TabUnfocused]		= Color::Gray;
		InStyle.Colors[ImGuiCol_TabUnfocusedActive] = Color::Gray;
		InStyle.Colors[ImGuiCol_TabSelectedOverline] = Color::BackgroundDark;

		//InStyle.Colors[ImGuiCol_Header] = Color::Coral;
		InStyle.Colors[ImGuiCol_Header]				= Color::Gray;
		InStyle.Colors[ImGuiCol_HeaderActive]		= Color::White;
		InStyle.Colors[ImGuiCol_HeaderHovered]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_DockingPreview]		= Color::Coral;
		InStyle.Colors[ImGuiCol_ResizeGrip]			= Color::Coral;
		InStyle.Colors[ImGuiCol_ResizeGripHovered]	= Color::CoralHover;
		InStyle.Colors[ImGuiCol_ResizeGripActive]	= Color::CoralActive;

		InStyle.Colors[ImGuiCol_FrameBg]			= Color::Gray;
		InStyle.Colors[ImGuiCol_FrameBgHovered]		= Color::CoralHover;
		InStyle.Colors[ImGuiCol_FrameBgActive]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_Separator]			= Color::Gray;
		InStyle.Colors[ImGuiCol_SeparatorHovered]	= Color::CoralHover;

		InStyle.Colors[ImGuiCol_Button]				= Color::Coral;
		InStyle.Colors[ImGuiCol_ButtonHovered]		= Color::CoralHover;
		InStyle.Colors[ImGuiCol_ButtonActive]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_SliderGrab]			= Color::White;
		InStyle.Colors[ImGuiCol_SliderGrabActive]	= Color::White;

		InStyle.Colors[ImGuiCol_CheckMark]			= Color::White;
		InStyle.Colors[ImGuiCol_NavHighlight]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_TableRowBg]			= Color::Gray;
		InStyle.Colors[ImGuiCol_TableRowBg]			= Color::BackgroundDark;


		InStyle.SeparatorTextBorderSize = 0.25f;
		InStyle.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
	}
} // namespace Luden::gui
