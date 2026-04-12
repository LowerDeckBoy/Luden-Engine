#pragma once

#include <Core/Time/Timer.hpp>

namespace Luden
{
	class Renderer;
}

namespace Luden::Panel
{
	class ConfigPanel
	{
	public:
		ConfigPanel() : m_Renderer(nullptr), m_Timer(nullptr) {}
		ConfigPanel(Renderer* pRenderer, Core::Timer* pTimer)
			: m_Renderer(pRenderer), m_Timer(pTimer) {}

		void Initialize(Renderer* pRenderer, Core::Timer* pTimer);

		void DrawPanel();

		void DrawDebugPanel();

		inline static int32 DisplayImageIndex = 0;
		inline static uint64 DisplayImageAddress = 0;

	private:
		Renderer* m_Renderer;
		Core::Timer* m_Timer;

	private:
		void DrawSceneConfig();
		void DrawSceneCameraConfig();
		void DrawPostProcessConfig();
		void DrawBloomConfig();
		void DrawAntiAliasingConfig();
		void DrawTonemappingConfig();
		void DrawAmbientOcclusionConfig();
		void DrawSkyConfig();
		void DrawProceduralSkyConfig();

		void DrawSpaceScreenReflectionsConfig();

	};
} // namespace Luden::Panel
