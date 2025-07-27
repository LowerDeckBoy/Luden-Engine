#pragma once

#include "GeometryPass.hpp"
#include "Scene/Scene.hpp"

namespace Luden
{
	class ShaderCompiler;

	class LightPass : public RenderPass
	{
	public:
		LightPass(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height);
		~LightPass();

		void Initialize(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, GeometryPass* pGeometryPass, uint32 Width, uint32 Height);

		//void Render(Scene* pScene, Frame& CurrentFrame);
		void Render(Scene* pScene, Frame& CurrentFrame, SceneCamera* pCamera);
		void RenderCompute(Scene* pScene, Frame& CurrentFrame, SceneCamera* pCamera);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		D3D12RenderTexture RenderTexture;

	private:
		// Private reference to GeometryPass.
		// For internal use only.
		GeometryPass* m_GeometryPass = nullptr;

		// Testing
		// Dispatch LightPass as Compute PSO instead of Vertex one.
		D3D12Pipeline ComputePSO;
		//D3D12ComputePipelineStateBuilder ComputePSO;
		bool bUseCompute = false;

	};
} // namespace Luden
