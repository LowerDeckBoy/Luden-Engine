#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	struct Frame;

	// TODO:
	// Pla
	class GeometryPass : public RenderPass
	{
	public:
		GeometryPass(D3D12RHI* pRHI, uint32 Width, uint32 Height);
		~GeometryPass();

		void Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height);

		void Release();

		void Resize(uint32 Width, uint32 Height) override;

		void Render(Scene* pScene, Frame& CurrentFrame);
		void Render(Scene* pScene, Frame& CurrentFrame, std::function<void()>  const& DrawFunction);

		//std::vector<FRenderTextures> RenderTextures;

		//D3D12RenderTexture* Depth;
		D3D12RenderTexture* BaseColor;
		D3D12RenderTexture* Normal;
		D3D12RenderTexture* MetallicRoughness;
		D3D12RenderTexture* Emissive;
		//D3D12RenderTexture* WorldPosition;

		D3D12Pipeline Pipeline;

	private:
		// Test
		std::vector<D3D12Descriptor*> m_RenderTargetHandles;

	};
} // namespace Luden
