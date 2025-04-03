#pragma once

#include "D3D12Device.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12CommandQueue.hpp"

#define SIZE_TO_KB(Size) Size			  * 1024
#define SIZE_TO_MB(Size) SIZE_TO_KB(Size) * 1024
#define SIZE_TO_GB(Size) SIZE_TO_MB(Size) * 1024

namespace Luden
{
	class D3D12Buffer;
	class D3D12Texture;
	class D3D12Resource;
	class D3D12RHI;

	enum EUploadType
	{
		Buffer,
		Texture
	};

	struct UploadResourceRequest
	{
		D3D12Resource*	Resource{};
		D3D12Resource*	UploadResource{};

		EUploadType		UploadType;
	};

	class D3D12UploadContext
	{
	public:
		static void Create(D3D12RHI* pD3D12RHI);

		static void Release();

		// Upload all pending requests.
		static void Upload();

		static void SingleUpload(UploadResourceRequest& Request);

		static void UploadTexture(D3D12Texture* pTexture, D3D12Resource* pUploadBuffer);

		static void UploadBuffer(D3D12Buffer* pBuffer, uint64 Size);
		static void UploadTexture(D3D12Texture* pTexture);

		static void PlaceRequest(D3D12Buffer* pBuffer, uint64 Size);
		static void PlaceRequest(D3D12Texture* pTexture, D3D12Resource* pUploadBuffer);

		static std::vector<UploadResourceRequest> PendingRequests;

		static D3D12CommandQueue*	UploadQueue;
		static D3D12CommandList*	CommandList;

	private:
		static D3D12RHI* m_D3D12RHI;

	};
} // namespace Luden
