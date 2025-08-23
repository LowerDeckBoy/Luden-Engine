#ifndef FILM_GRAIN_HLSL
#define FILM_GRAIN_HLSL

#include "FilmGrain_RS.hlsli"

#define DISPATCH_BLOCK 8

[RootSignature(FILM_GRAIN_RS)]
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
}

#endif // FILM_GRAIN_HLSL
