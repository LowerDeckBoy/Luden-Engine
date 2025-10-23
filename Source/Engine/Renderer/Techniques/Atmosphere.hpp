#pragma once

#include "../RenderPass.hpp"
#include <DirectXMath.h>

namespace Luden
{
	class ShaderCompiler;
	class SceneCamera;

	// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
	// https://cpp-rendering.io/sky-and-atmosphere-rendering/
	// https://github.com/kentril0/OpenGL_Atmospheric_Scattering/tree/master
	class Atmosphere : public RenderPass
	{
	public:
		Atmosphere(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~Atmosphere();

		void Render(Frame& CurrentFrame, SceneCamera* pCamera, uint32 SceneImageIndex, uint32 WorldPositionIndex, uint32 DepthIndex, DirectX::XMFLOAT3 SunPosition);
		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		//AtmosphereParameters Parameters;

		struct Constants
		{
			uint32 SceneImageIndex;
			uint32 OutputImageIndex;
			uint32 WorldPositionIndex;
			uint32 DepthIndex;

			DirectX::XMFLOAT4 CameraPosition;		// TODO: repack it later
			DirectX::XMFLOAT4 SunPosition;			// TODO: repack it later

			float Brightness;						// Intensity

			float RadiusAtmosphere;					// Outer Radius
			float RadiusPlanetery;					// Inner Radius
			float padding = 0.0f;
			DirectX::XMFLOAT3 ScatteringCoefficiencyRayleigh;	// Kr
			float ScatteringCoefficiencyMie;		// Km
			float ScaleHeightRayleigh;				// Hr
			float ScaleHeightMie;					// Hm
			float ScatteringDirection;				// Gm
		} Parameters;

		D3D12RenderTexture DebugRenderTarget;

		// https://docs.lightwave3d.com/2025/physical-sky-hosek-wilkie.html
		// https://github.com/TheRealMJP/DXRPathTracer/blob/master/SampleFramework12/v1.02/Graphics/Skybox.h
		struct SkyConstants
		{
			DirectX::XMFLOAT4 CameraPosition;		// TODO: repack it later
			DirectX::XMFLOAT4 SunPosition;			// TODO: repack it later

			DirectX::XMFLOAT4 SurfaceAlbedo;		// TODO: repack it later
			DirectX::XMFLOAT4 ScatteringColor;		// TODO: repack it later
			DirectX::XMFLOAT4 TintColor;			// TODO: repack it later

			float Luminance;
			float Turbidity;

		} SkyParameters;

	private:

	};
} // namespace Luden
