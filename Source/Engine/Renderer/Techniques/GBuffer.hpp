#pragma once

#include "../RenderPass.hpp"

namespace Luden
{
	// BaseColor:	r8g8b8a8
	// Normal:		r11g11b10
	// 

	class GBuffer : public RenderPass
	{
	public:
		GBuffer(D3D12RHI* pRHI);

		void Resize(uint32 Width, uint32 Height) override;

	private:

	};
} // namespace Luden
