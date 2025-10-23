#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class Scattering : public RenderPass
	{
	public:
		Scattering(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~Scattering();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, DirectX::XMFLOAT3 SunPosition);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		D3D12RenderTexture RenderTarget;

		struct
		{
			uint32 SceneImageIndex;
			uint32 OutputImageIndex;
			float IlluminationDecay = 1.25f;
			float Weight = 1.25f;
			DirectX::XMFLOAT3 ScreenLightPosition;
			float padding;
		} Parameters;

	private:

	};

} // namespace Luden
