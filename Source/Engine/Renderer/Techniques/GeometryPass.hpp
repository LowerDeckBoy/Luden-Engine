#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	struct Frame;

	class GeometryPass : public RenderPass
	{
	public:
		GeometryPass(D3D12RHI* pRHI, uint32 Width, uint32 Height);
		~GeometryPass();

		void Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height);

		void Release() override;

		void Resize(uint32 Width, uint32 Height) override;

		void Render(Frame& CurrentFrame, std::function<void()>  const& DrawFunction);

		D3D12RenderTexture BaseColor;
		D3D12RenderTexture Normal;
		D3D12RenderTexture MetallicRoughness;
		D3D12RenderTexture Emissive;

	private:
		// For internal use only.
		std::vector<D3D12Descriptor*> m_RenderTargetHandles;

	};
} // namespace Luden
