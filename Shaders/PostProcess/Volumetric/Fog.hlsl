#ifndef FOG_HLSL
#define FOG_HLSL

#define DISPATCH_BLOCK 8


// https://ijdykeman.github.io/graphics/simple_fog_shader
[numthreads(DISPATCH_BLOCK, DISPATCH_BLOCK, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
}

#endif // FOG_HLSL
