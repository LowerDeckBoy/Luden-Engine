#ifndef TAA_HLSL
#define TAA_HLSL

#define DISPATCH_BLOCK 8

// https://github.com/hadryansalles/Luz/blob/main/source/Shaders/taa.comp
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(int3 DispatchThreadID : SV_DispatchThreadID)
{
}

#endif // TAA_HLSL
