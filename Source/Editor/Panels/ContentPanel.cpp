#include "../Colors.hpp"
#include "../Components/Tooltip.hpp"
#include "../Editor.hpp"
#include "ContentPanel.hpp"
#include <Core/File.hpp>


namespace Luden::Panel
{
	// Temporal
	std::string AssetDirectory = "../../Assets";

	ContentPanel::ContentPanel()
		: m_CurrentDirectory(AssetDirectory)
	{
		m_CurrentDirectory = std::filesystem::current_path().parent_path().parent_path().append("Assets");
	}

	void ContentPanel::DrawPanel()
	{
		ImGui::Begin("Content Hierarchy");

		const std::string& directoryString = m_CurrentDirectory.string();
		ImGui::Text("Current directory: %s", directoryString.c_str());

		ImGui::End();

		ImGui::Begin("Content");

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });

		static float padding = 25.0f;
		static float imageSize = 64.0f;
		float cellSize = imageSize + padding;

		const ImVec2 thumbnailSize = { imageSize, imageSize };

		const auto& panelWidth = ImGui::GetContentRegionAvail().x;
		int32 columnsCount = (int32)(panelWidth / cellSize);
		if (columnsCount < 1.0f) columnsCount = 1;
		
		if (ImGui::BeginTable("Assets", columnsCount))
		{
			ImGui::TableNextColumn();

			gui::OnItemHover("Move to parent directory.");
			ImGui::Button("...", thumbnailSize);
			if (ImGui::IsItemHovered())
			{
				ImGui::PushStyleColor(ImGuiCol_Border, gui::Color::Gray);
				ImGui::SetTooltip("Move to parent directory.");
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
			ImGui::Text("...");

			ImGui::TableNextColumn();

			for (const auto& path : std::filesystem::directory_iterator(Filepath(m_CurrentDirectory)))
			{
				std::string str = path.path().filename().string();

				if (path.is_directory())
				{
					ImGui::ImageButton(str.c_str(), (ImTextureID)Luden::Editor::EditorDirectoryTexture->ShaderResourceHandle.GpuHandle.ptr, thumbnailSize);

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.path().filename();
					}

					ImGui::Text(str.c_str());
				}
				else
				{
					ImGui::ImageButton(str.c_str(), (ImTextureID)Luden::Editor::EditorFileTexture->ShaderResourceHandle.GpuHandle.ptr, thumbnailSize);

					if (ImGui::BeginDragDropSource())
					{
						std::string relative = AssetDirectory + "/";
						relative.append(std::filesystem::relative(path, AssetDirectory).string());

						// Without adding 1 to length some files cannot be loaded.
						ImGui::SetDragDropPayload("PAYLOAD_ITEM", relative.data(), (relative.length() + 1) * sizeof(char), ImGuiCond_Once);
	
						ImGui::EndDragDropSource();
						
					}

					ImGui::Text(str.c_str());
					gui::OnItemHover(str);
				}

				ImGui::TableNextColumn();
			}

			ImGui::EndTable();
		}
		
		ImGui::PopStyleColor();

		ImGui::End();
	}

} // namespace Luden::Panel
