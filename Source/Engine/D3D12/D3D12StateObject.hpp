#pragma once

#include "D3D12Shader.hpp"
#include <Core/RefPtr.hpp>
#include <D3D12AgilitySDK/d3d12.h>
#include <D3D12AgilitySDK/d3dx12/d3dx12_state_object.h>
#include "D3D12ShaderTable.hpp"

namespace Luden
{
	class D3D12Device;
	class D3D12RootSignature;

	// Used in Raytracing pipelines.
	class D3D12StateObject
	{
	public:
		D3D12StateObject();
		~D3D12StateObject();

		Ref<ID3D12StateObject>& GetHandle()		{ return m_StateObject;		  }
		ID3D12StateObject*		GetHandleRaw()	{ return m_StateObject.Get(); }

		Ref<ID3D12StateObjectProperties>& GetProperties() { return m_StateObjectProperties; }

		void Release();

		D3D12Shader* RayGen;
		D3D12Shader* Miss;
		D3D12Shader* ClosestHit;

		uint32 MaxRecursion		= 1;
		uint32 PayloadSize		= 12;
		uint32 AttributeSize	= D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

		Ref<ID3D12StateObject>				m_StateObject;
		Ref<ID3D12StateObjectProperties>	m_StateObjectProperties;
	private:
		//CD3DX12_STATE_OBJECT_DESC			m_StateObjectDesc{};

	};

	struct FHitGroup
	{
		std::string_view Name = "";
		std::string_view ClosestHitName = "";
		std::string_view IntersectionName = "";
		std::string_view AnyHitName = "";
		D3D12_HIT_GROUP_TYPE Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
	};

	class D3D12StateObjectBuilder
	{
		friend class D3D12StateObject;
	public:
		D3D12StateObjectBuilder();
		//D3D12StateObjectBuilder(D3D12StateObject& OutStateObject, D3D12Device* pDevice);

		void AddRayGen(D3D12Shader* pShader, std::vector<LPCWSTR> Exports);
		void AddMiss(D3D12Shader* pShader, std::vector<LPCWSTR> Exports);
		void AddClosestHit(D3D12Shader* pShader, std::vector<LPCWSTR> Exports);

		void AddHitGroup(FHitGroup HitGroup);

		void SetMaxRayRecursion(uint32 MaxRecursion);
		void SetPayloadSize(uint32 PayloadSize);

		void SetStateObjectType(D3D12_STATE_OBJECT_TYPE Type);

		void SetGlobalRootSignature(D3D12RootSignature* pRootSignature);
		void SetGlobalRootSignature(D3D12RootSignature* pRootSignature, std::vector<LPCWSTR> Exports);

		void Build(D3D12Device* pDevice, D3D12StateObject& Output);

		D3D12StateObject m_Output;
	private:
		CD3DX12_STATE_OBJECT_DESC m_Desc{};

		uint32 m_MaxRecursion	= 1;
		uint32 m_PayloadSize	= sizeof(DirectX::XMFLOAT3);
		uint32 m_AttributeSize  = sizeof(DirectX::XMFLOAT2);
		
		std::vector<FHitGroup> m_HitGroups;

	};
} // namespace Luden