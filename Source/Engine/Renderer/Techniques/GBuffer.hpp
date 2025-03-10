#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	// BaseColor:	r8g8b8a8
	// Normal:		r11g11b10
	// 

	class GBuffer : public RenderPass
	{
	private:
		struct FRenderTextures
		{
			D3D12RenderTexture* BaseColor;
			D3D12RenderTexture* Normal;
			D3D12RenderTexture* Emissive;
			D3D12RenderTexture* WorldPosition;
		};
	public:
		GBuffer(D3D12RHI* pRHI);

		void Resize(uint32 Width, uint32 Height) override;

		std::vector<FRenderTextures> RenderTextures;

	private:
		

	};
} // namespace Luden
