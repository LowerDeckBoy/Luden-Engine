#pragma once

namespace Luden::Panel
{
	class ContentPanel
	{
	public:
		ContentPanel();
		~ContentPanel() = default;

		void DrawPanel();

	private:
		Filepath m_CurrentDirectory = "";

	};
} // namespace Luden::Panel
