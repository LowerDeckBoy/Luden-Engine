#ifndef INDIRECT_HLSL
#define INDIRECT_HLSL

#include "Indirect_RS.hlsli"

struct IndirectArgs
{
	uint ArgBufferIndex;
	uint ArgBufferIndex1;
	uint ArgBufferIndex2;
	uint ArgBufferIndex3;
	uint ArgBufferIndex4;
	uint ArgBufferIndex5;
	uint ArgBufferIndex6;
	uint ArgBufferIndex7;
	uint ArgBufferIndex8;
	uint ArgBufferIndex9;
	uint ArgBufferIndex10;
	uint ArgBufferIndex11;
	uint ArgBufferIndex12;
	uint ArgBufferIndex13;
};

ConstantBuffer<IndirectArgs> Args : register(b1);

[RootSignature(INDIRECT_ROOT_SIG)]
[numthreads(128, 1, 1)]
void CSMain(uint3 GroupID : SV_GroupID, uint GroupIndex : SV_GroupIndex)
{
}

#endif // INDIRECT_HLSL
