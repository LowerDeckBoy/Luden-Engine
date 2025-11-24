#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	class TAA : public RenderPass
	{
	public:
		TAA(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler);
		~TAA();

		void Render(Frame& CurrentFrame, uint32 SceneImageIndex, uint32 OutputImageIndex);

		void Release() override;
		void Resize(uint32 Width, uint32 Height) override {}
	
	private:

	};
} // namespace Luden
