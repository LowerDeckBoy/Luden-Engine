#ifndef CHROMATIC_ABERRATION_HLSLI
#define CHROMATIC_ABERRATION_HLSLI

#define GROUP_SIZE 8

[numthreads(GROUP_SIZE, GROUP_SIZE, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{

	//refract()

}

#endif // CHROMATIC_ABERRATION_HLSLI
