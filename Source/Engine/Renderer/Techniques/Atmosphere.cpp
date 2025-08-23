#include "Asset/ShaderCompiler.hpp"
#include "Atmosphere.hpp"

namespace Luden
{
	Atmosphere::Atmosphere(D3D12RHI* pD3D12RHI, ShaderCompiler* pShaderCompiler, uint32 Width, uint32 Height)
		: RenderPass(pD3D12RHI)
	{

		DebugRenderTarget.Create(pD3D12RHI->Device, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT, DefaultClearColor);



		// Temp
		Parameters.ScatteringCoefficiencyRayleigh = DirectX::XMFLOAT3(5.802f * 1e-6f, 13.558f * 1e-6f, 33.100f * 1e-6f);
	}

	Atmosphere::~Atmosphere()
	{
	}

	void Atmosphere::Render(Frame& CurrentFrame)
	{
		const auto renderTimeBegin = Time::GetTimestamp();



		RenderTime = Time::GetDurationInMiliseconds(renderTimeBegin);
	}

	void Atmosphere::Resize(uint32 Width, uint32 Height)
	{
		DebugRenderTarget.Resize(Width, Height);
	}

} // namespace Luden
