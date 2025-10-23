#ifndef HBAO_HLSL
#define HBAO_HLSL

#include "HBAO_RS.hlsli"

#define DISPATCH_BLOCK 16

[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{

}

#endif // HBAO_HLSL
