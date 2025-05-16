#pragma once

namespace Luden::Panel
{
	// TODO:
	// - Make tree structure of project directories.
	// - Determine display icons by file extensions.
	class ContentPanel
	{
		friend class Editor;
	public:
		ContentPanel();
		~ContentPanel() = default;

		void DrawPanel();

	private:
		Filepath m_CurrentDirectory = "";

	};
} // namespace Luden::Panel
