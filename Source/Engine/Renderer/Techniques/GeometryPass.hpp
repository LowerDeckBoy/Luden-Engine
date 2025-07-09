#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class ShaderCompiler;
	class Scene;
	class SceneCamera;
	struct Frame;

	class GeometryPass : public RenderPass
	{
	public:
		GeometryPass(D3D12RHI* pRHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height);
		~GeometryPass();

		void Initialize(D3D12RHI* pRHI, uint32 Width, uint32 Height);

		void CreatePipeline(ShaderCompiler* pShaderCompiler);

		void Release() override;

		void Resize(uint32 Width, uint32 Height) override;

		void Render(Frame& CurrentFrame, std::function<void()> const& DrawFunction);
		void Render(Scene* pScene, SceneCamera* pCamera, Frame& CurrentFrame);

		D3D12RenderTexture BaseColor;
		D3D12RenderTexture Normal;
		D3D12RenderTexture MetallicRoughness;
		D3D12RenderTexture Emissive;
		D3D12RenderTexture WorldPosition;

	private:
		// For internal use only.
		std::vector<D3D12Descriptor*> m_RenderTargetHandles;

		D3D12Pipeline BlendPipelineState;

	};
} // namespace Luden
