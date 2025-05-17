#pragma once

namespace Luden::Panel
{
	// TODO:
	// - Make tree structure of project directories.
	// - Determine display icons by file extensions.
	class ContentBrowserPanel
	{
		friend class Editor;
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void DrawPanel();

	private:
		Filepath m_CurrentDirectory = "";

	};
} // namespace Luden::Panel
