#pragma once

#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <array>

namespace Luden
{
	class D3D12Device;
	class D3D12Descriptor;
	class D3D12DescriptorHeap;
	class D3D12Viewport;
	class D3D12Resource;
	class D3D12RootSignature;
	class D3D12PipelineState;
	class D3D12ConstantBuffer;
	class D3D12Buffer;
	class D3D12Texture;

	constexpr std::array<float, 4> DefaultClearColor		= { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr std::array<float, 4> RenderTargetClearColor	= { 0.0f, 0.0f, 0.0f, 0.0f };

	class D3D12CommandList
	{
	public:
		D3D12CommandList(D3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE CommandListType);
		~D3D12CommandList();

		Ref<ID3D12GraphicsCommandList10>&	GetHandle()		{ return m_GraphicsCommandList; }
		ID3D12GraphicsCommandList10*		GetHandleRaw()	{ return m_GraphicsCommandList.Get(); }
		Ref<ID3D12CommandAllocator>&		GetAllocator()	{ return m_CommandAllocator; }

		HRESULT Open();
		HRESULT Close();

		// Reset CommandList and it's Allocator without signaling CommandList as Open.
		void Flush();

		bool IsOpen() const { return bIsOpen; }
		
		// TODO: Add Sampler Heap binding.
		// nullptr as default.
		void SetDescriptorHeap(D3D12DescriptorHeap* pDescriptorHeap);

		void SetViewport(D3D12Viewport* pViewport);

		// Single Resource transition.
		// Before state is taken from pResource itself.
		void ResourceTransition(D3D12Resource* pResource, D3D12_RESOURCE_STATES After);
		// Multiple Resources transition.
		void ResourcesTransition(const std::vector<std::pair<D3D12Resource*, D3D12_RESOURCE_STATES>>& pResources);

		// Either Buffer to Buffer or Texture to Texture.
		void CopyResource(D3D12Resource* pSource, D3D12Resource* pDestination);

		void CopyBufferToBuffer(D3D12Buffer* pSource, D3D12Buffer* pDestination);
		void CopyBufferToBuffer(D3D12Resource* pSource, D3D12Resource* pDestination, void* pData, uint64 Size);

		void CopyTextureToTexture(D3D12Resource* pSource, D3D12Resource* pDestination, D3D12_SUBRESOURCE_DATA* Subresource);
		void CopyTextureToTexture(D3D12Texture* pSource, D3D12Texture* pDestination, D3D12_SUBRESOURCE_DATA Subresource);

		void SetRootSignature(D3D12RootSignature* pRootSignature);
		void SetGraphicsRootSignature(D3D12RootSignature* pRootSignature);
		void SetComputeRootSignature(D3D12RootSignature* pRootSignature);

		void SetPipelineState(D3D12PipelineState* pPipelineState);
		//void SetRaytracingPipeline(D3D12PipelineState* pPipelineState);

		void ResolveSubresource(D3D12Resource* DestResource, uint32 DestSubresource, D3D12Resource* SourceResource, uint32 SourceSubresource, DXGI_FORMAT Format);

		void ClearDepthStencilView(D3D12Descriptor& DepthStencilView);

		void SetRenderTarget(D3D12Descriptor& RenderTargetView);
		void SetRenderTarget(D3D12Descriptor& RenderTargetView, D3D12Descriptor& DepthStencilView);
		void SetRenderTargets(const std::vector<D3D12Descriptor*>& RenderTargetViews, D3D12Descriptor& DepthStencilView);

		void ClearRenderTarget(D3D12Descriptor& RenderTargetView, std::array<float, 4> ClearColor = DefaultClearColor);
		void ClearRenderTargets(const std::vector<D3D12Descriptor*>& RenderTargetViews, std::array<float, 4> ClearColor = DefaultClearColor);

		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);

		void PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset = 0);
		void PushRootSRV(uint32 Slot, uint64 Address);

		void DispatchMesh(uint32 DispatchThreadX, uint32 DispatchThreadY, uint32 DispatchThreadZ);

		void Draw(uint32 VertexCount);
		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex);

		//void SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW* pIndexBufferView);
		void SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW IndexBufferView);
		void SetIndexBuffer(D3D12Buffer* pIndexBuffer);
		void SetConstantBuffer(uint32 RegisterSlot, D3D12ConstantBuffer* pConstantBuffer);
		
	private:
		Ref<ID3D12GraphicsCommandList10>	m_GraphicsCommandList;
		Ref<ID3D12CommandAllocator>			m_CommandAllocator;
		D3D12_COMMAND_LIST_TYPE				m_CommandListType;
		
		bool bIsOpen = false;

	}; // class D3D12CommandList
	
} // namespace Luden
