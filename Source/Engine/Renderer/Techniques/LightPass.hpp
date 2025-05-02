#pragma once

#include "GeometryPass.hpp"
#include "Scene/Scene.hpp"

namespace Luden
{
	class LightPass : public RenderPass
	{
	public:
		LightPass(D3D12RHI* pD3D12RHI, GeometryPass* pGeometryPass, uint32 Width, uint32 Height);
		~LightPass();

		void Initialize(D3D12RHI* pD3D12RHI, GeometryPass* pGeometryPass, uint32 Width, uint32 Height);

		void Render(Scene* pScene, Frame& CurrentFrame);

		void Resize(uint32 Width, uint32 Height) override;
		void Release() override;

		D3D12RenderTexture RenderTexture;

	private:
		// Private reference to GeometryPass.
		// For internal use only.
		GeometryPass* m_GeometryPass = nullptr;

	};
} // namespace Luden
