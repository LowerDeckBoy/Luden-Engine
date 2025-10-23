#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	class SceneCamera;
	
	// https://github.com/mateeeeeee/Adria/blob/master/Adria/Resources/Shaders/CommonResources.hlsli
	class SSR : public RenderPass
	{
	public:
		SSR(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~SSR();

		void Render(Frame* CurrentFrame, uint32 SceneImageIndex, uint32 NormalsIndex, uint32 RoughnessIndex, SceneCamera* pCamera);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		struct
		{
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX InversedProjection;

			uint32 InputImageIndex;
			uint32 OutputImageIndex;
			uint32 NormalIndex;
			uint32 RoughnessIndex;
			uint32 DepthIndex;
			float  RaySteps		= 1.6f;
			float  RayThreshold	= 2.0f;
			float  padding;

		} Parameters;

		D3D12RenderTexture RenderTarget;

	private:

	};
} // namespace Luden
