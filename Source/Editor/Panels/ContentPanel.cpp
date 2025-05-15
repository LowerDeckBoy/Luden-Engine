#include <Core/File.hpp>
#include "ContentPanel.hpp"
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <FontAwsome6/IconsFontAwesome6.h>


namespace Luden::Panel
{
	// Temporal
	std::string AssetDirectory = "../../Assets";
	//std::string AssetDirectory = "Luden/Assets";
	//const char* AssetDirectory = "Assets/";

	ContentPanel::ContentPanel()
		: m_CurrentDirectory(AssetDirectory)
	{
		m_CurrentDirectory = std::filesystem::current_path().parent_path().parent_path().append("Assets");
		//m_CurrentDirectory += "Assets";
	}

	void ContentPanel::DrawPanel()
	{
		ImGui::Begin("Content Hierarchy");

		std::string directoryString = m_CurrentDirectory.string();
		ImGui::Text("Current directory: %s", directoryString.c_str());

		ImGui::End();

		ImGui::Begin("Content");

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });

		if (ImGui::Button("...", { 50, 50 }))
		{
			if (m_CurrentDirectory.has_parent_path())
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				
			}
		}
		
		ImGui::SameLine();

		for (auto& path : std::filesystem::directory_iterator(Filepath(m_CurrentDirectory)))
		{
			std::string str = path.path().filename().string();
			auto availableSize = ImGui::GetContentRegionAvail();

			
			if (path.is_directory())
			{
				if (availableSize.x - 50 > 55.0)
				{
					//ImGui::Text("%s", str.c_str());
					//ImGui::PushID(str.c_str());
					
					//if (ImGui::Button(ICON_FA_FOLDER"", {50, 50}))
					if (ImGui::Button(str.c_str(), { 50, 50 }))
					{
						m_CurrentDirectory /= path.path().filename();
					}
					
					//ImGui::PopID();
					ImGui::SameLine();
				}
				else
				{
					
					//ImGui::Text("%s", str.c_str());
					//ImGui::PushID(str.c_str());
					if (ImGui::Button(str.c_str(), { 50, 50 }))
					{
						m_CurrentDirectory /= path.path().filename();
					}
					//ImGui::PopID();
				}			
			}
			else
			{
				if (availableSize.x - 50 > 55.0)
				{
					ImGui::Button(str.c_str(), { 50, 50 });
					ImGui::SameLine();
				}
				else
				{
					ImGui::Button(str.c_str(), { 50, 50 });
				}
				//ImGui::PushID(str.c_str());
				//ImGui::Button(ICON_FA_FILE"", { 50, 50 });
				//ImGui::PopID();
			}
		}
		ImGui::PopStyleColor();

		ImGui::End();
	}
} // namespace Luden::Panel
