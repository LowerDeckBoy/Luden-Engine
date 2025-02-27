#pragma once

#include "D3D12/D3D12RHI.hpp"
#include <Core/Types.hpp>

namespace Luden
{
	// Base class for Render Pass functionality.
	// Makes it easier to store and iterate over different techniques,
	// like resizing and releasing.
	class RenderPass
	{
	public:
		RenderPass(D3D12RHI* pRHI)
			: m_RHI(pRHI) 
		{ }

		virtual void Resize(uint32 Width, uint32 Height) = 0;

	protected:
		D3D12RHI* m_RHI;
		// RootSignature
		// PipelineState

	};
} // namespace Luden
