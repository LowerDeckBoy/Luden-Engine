#ifndef INDIRECT_HLSL
#define INDIRECT_HLSL

#include "Indirect_RS.hlsli"

[RootSignature(INDIRECT_ROOT_SIG)]
[numthreads(128, 1, 1)]
void CSMain(uint3 GroupID : SV_GroupID, uint GroupIndex : SV_GroupIndex)
{
}

#endif // INDIRECT_HLSL
