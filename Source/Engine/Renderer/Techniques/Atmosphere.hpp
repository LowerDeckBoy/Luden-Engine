#pragma once

#include "../RenderPass.hpp"
#include <DirectXMath.h>

namespace Luden
{
	class ShaderCompiler;

	// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
	// https://cpp-rendering.io/sky-and-atmosphere-rendering/
	// https://github.com/kentril0/OpenGL_Atmospheric_Scattering/tree/master
	class Atmosphere : public RenderPass
	{
	public:
		Atmosphere(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~Atmosphere();

		void Render(Frame& CurrentFrame);
		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		//AtmosphereParameters Parameters;

		struct Constants
		{
			DirectX::XMFLOAT4 CameraPosition;		// TODO: repack it later
			DirectX::XMFLOAT4 SunPosition;			// TODO: repack it later

			float Brightness;						// Intensity

			float RadiusAtmosphere;					// Outer Radius
			float RadiusPlanetery;					// Inner Radius
			DirectX::XMFLOAT3 ScatteringCoefficiencyRayleigh;	// Kr
			float ScatteringCoefficiencyMie;		// Km
			float ScaleHeightRayleigh;				// Hr
			float ScaleHeightMie;					// Hm
			float ScatteringDirection;				// Gm
		} Parameters;

		D3D12RenderTexture DebugRenderTarget;

	private:

	};
} // namespace Luden
