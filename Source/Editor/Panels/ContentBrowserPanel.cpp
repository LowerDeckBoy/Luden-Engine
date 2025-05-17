#include "../Colors.hpp"
#include "../Components/Tooltip.hpp"
#include "../Editor.hpp"
#include "ContentBrowserPanel.hpp"


namespace Luden::Panel
{
	enum class EFileAssetExt
	{
		File,
		GLTF,
		GLB,
		FBX,
		OBJ,
		PNG,
		JPG,
		JPEG,
		BIN
	};

	EFileAssetExt StringToExt(const std::string& String);

	static uint64 GetFileIconToExtension(std::string StringExt);

	// Temporal
	std::string AssetDirectory = "../../Assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(AssetDirectory)
	{
		m_CurrentDirectory = std::filesystem::current_path().parent_path().parent_path().append("Assets");
	}

	void ContentBrowserPanel::DrawPanel()
	{
		ImGui::Begin("Content Hierarchy");

		const std::string& directoryString = m_CurrentDirectory.string();
		ImGui::Text("Current directory: %s", directoryString.c_str());

		ImGui::End();

		ImGui::Begin("Content");

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 0.1f });

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
				const std::string filename = path.path().filename().string();

				if (path.is_directory())
				{
					ImGui::ImageButton(filename.c_str(), (ImTextureID)Luden::Editor::EditorDirectoryTexture->ShaderResourceHandle.GpuHandle.ptr, thumbnailSize);

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.path().filename();
					}

					ImGui::Text(filename.c_str());
				}
				else
				{
					ImGui::ImageButton(filename.c_str(), (ImTextureID)GetFileIconToExtension(File::GetExtension(path)), thumbnailSize);

					if (ImGui::BeginDragDropSource())
					{
						const std::string relative = AssetDirectory + "/" + File::GetRelativePath(path, AssetDirectory);

						// Without adding 1 to length some files cannot be loaded.
						ImGui::SetDragDropPayload("PAYLOAD_ITEM", relative.data(), (relative.length() + 1) * sizeof(char), ImGuiCond_Once);
	
						ImGui::EndDragDropSource();
						
					}

					ImGui::Text(filename.c_str());
					gui::OnItemHover(filename);
				}

				ImGui::TableNextColumn();
			}

			ImGui::EndTable();
		}
		
		ImGui::PopStyleColor();

		ImGui::End();
	}

	EFileAssetExt StringToExt(const std::string& String)
	{
		if		(String.compare(".gltf") == 0) { return Luden::Panel::EFileAssetExt::GLTF; }
		else if (String.compare(".glb")  == 0) { return Luden::Panel::EFileAssetExt::GLB;  }
		else if (String.compare(".fbx")  == 0) { return Luden::Panel::EFileAssetExt::FBX;  }
		else if (String.compare(".obj")  == 0) { return Luden::Panel::EFileAssetExt::OBJ;  }
		else if (String.compare(".png")  == 0) { return Luden::Panel::EFileAssetExt::PNG;  }
		else if (String.compare(".jpg")  == 0) { return Luden::Panel::EFileAssetExt::JPG;  }
		else if (String.compare(".jpeg") == 0) { return Luden::Panel::EFileAssetExt::JPEG; }
		else if (String.compare(".bin") == 0)  { return Luden::Panel::EFileAssetExt::BIN; }
		
		return Luden::Panel::EFileAssetExt::File;
	}

	uint64 GetFileIconToExtension(std::string StringExt)
	{
		const auto ext = StringToExt(StringExt.data());

		switch (ext)
		{
		case EFileAssetExt::GLTF:	return Luden::Editor::EditorGLTFTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::GLB:	return Luden::Editor::EditorGLBTexture->ShaderResourceHandle.GpuHandle.ptr;
		//case EFileAssetExt::FBX:	return Luden::Editor::EditorFBXTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::OBJ:	return Luden::Editor::EditorOBJTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::PNG:	return Luden::Editor::EditorPNGTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::JPG:	return Luden::Editor::EditorJPGTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::JPEG:	return Luden::Editor::EditorJPEGTexture->ShaderResourceHandle.GpuHandle.ptr;
		case EFileAssetExt::BIN:	return Luden::Editor::EditorBINTexture->ShaderResourceHandle.GpuHandle.ptr;

		default:					return Luden::Editor::EditorFileTexture->ShaderResourceHandle.GpuHandle.ptr;
		}
	}

} // namespace Luden::Panel
