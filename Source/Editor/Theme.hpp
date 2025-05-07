#pragma once

#include "Colors.hpp"

namespace Luden::gui
{
	inline void DarkTheme(ImGuiStyle& InStyle)
	{
		ImGui::StyleColorsDark(&InStyle);

		InStyle.WindowRounding		= 1.0f;
		InStyle.WindowBorderSize	= 0.0f;
		InStyle.FrameRounding		= 1.0f;
		InStyle.FrameBorderSize		= 0.0f;
		InStyle.ChildBorderSize		= 0.0f;

		// Tooltip
		InStyle.PopupBorderSize		= 0.15f;
		InStyle.PopupRounding		= 1.0f;

		InStyle.Colors[ImGuiCol_Text]				= Color::White;
		InStyle.Colors[ImGuiCol_WindowBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_MenuBarBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_DockingEmptyBg]		= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_Border]				= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_BorderShadow]		= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_PopupBg]			= Color::BackgroundDark;

		InStyle.Colors[ImGuiCol_TitleBg]			= Color::BackgroundDark;
		InStyle.Colors[ImGuiCol_TitleBgActive]		= Color::BackgroundDarkActive;
		InStyle.Colors[ImGuiCol_TitleBgCollapsed]	= Color::CoralHover;

		InStyle.Colors[ImGuiCol_Tab]				= Color::Gray;
		InStyle.Colors[ImGuiCol_TabActive]			= Color::CoralActive;
		InStyle.Colors[ImGuiCol_TabHovered]			= Color::CoralHover;
		InStyle.Colors[ImGuiCol_TabUnfocused]		= Color::Gray;
		InStyle.Colors[ImGuiCol_TabUnfocusedActive] = Color::Gray;
		InStyle.Colors[ImGuiCol_TabSelectedOverline] = Color::BackgroundDark;

		InStyle.Colors[ImGuiCol_Header]				= Color::Gray;
		InStyle.Colors[ImGuiCol_HeaderActive]		= Color::White;
		InStyle.Colors[ImGuiCol_HeaderHovered]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_DockingPreview]		= Color::Coral;
		InStyle.Colors[ImGuiCol_ResizeGrip]			= Color::Coral;
		InStyle.Colors[ImGuiCol_ResizeGripHovered]	= Color::Coral;
		InStyle.Colors[ImGuiCol_ResizeGripActive]	= Color::Coral;

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

		InStyle.Colors[ImGuiCol_CheckMark]			= Color::Coral;
		InStyle.Colors[ImGuiCol_NavHighlight]		= Color::CoralActive;

		InStyle.Colors[ImGuiCol_TableRowBg]			= Color::Gray;
		
		InStyle.SeparatorTextBorderSize = 0.25f;
		InStyle.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
	}
} // namespace Luden::gui
