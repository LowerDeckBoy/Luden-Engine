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

		void Render(Frame* CurrentFrame, uint32 SceneImageIndex, uint32 NormalsIndex, uint32 RoughnessIndex, uint32 WorldPositionIndex, uint32 DepthIndex, SceneCamera* pCamera);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		struct
		{
			//DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
			DirectX::XMMATRIX InversedView;
			DirectX::XMMATRIX InversedProjection;

			uint32 InputImageIndex;
			uint32 OutputImageIndex;
			uint32 NormalIndex;
			uint32 RoughnessIndex;
			uint32 WorldPositionIndex;
			uint32 DepthIndex;

		} Parameters;

		D3D12RenderTexture RenderTarget;

	private:

	};
} // namespace Luden
