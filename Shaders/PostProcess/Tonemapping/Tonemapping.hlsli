#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

#include "../../Common/Bindless.hlsli"
#include "../../Common/Color.hlsli"

namespace Tonemapping
{
	static const uint TypeACESSimple		= 0;
	static const uint TypeACESFilm			= 1;
	static const uint TypeAMD				= 2;
	static const uint TypeAgX				= 3;
	static const uint TypeAgXPunchy			= 4;
	static const uint TypeAgXGolden			= 5;
	static const uint TypeReinhard			= 6;
	static const uint TypeGammaCorrection	= 7;
	static const uint TypeUncharted2		= 8;
	static const uint TypeHable				= 9;

	float3 TonemapReinhard(float3 Color)
	{
		const float luminance = GetLuminance(Color);
		const float reinhard = luminance / (1.0f + luminance);
		
		return saturate(Color * (reinhard / luminance));
	}
	
	float3 TonemapGammaCorrection(float3 Color)
	{
		float3 output = pow(Color, 0.4545454545f);
		
		return saturate(output);
	}
	
	float3 TonemapUncharted2(float3 Color)
	{
		const float A = 0.15f;
		const float B = 0.50f;
		const float C = 0.10f;
		const float D = 0.20f;
		const float E = 0.02f;
		const float F = 0.30f;

		return saturate(((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F);
	}

	
	float3 TonemapACESSimple(float3 Color)
	{
		const float a = 2.51f;
		const float b = 0.03f;
		const float c = 2.43f;
		const float d = 0.59f;
		const float e = 0.14f;
		
		return saturate((Color * (a * Color + b)) / (Color * (c * Color + d) + e));
	}
	
	float3 TonemapACESFilm(float3 Color)
	{
		static const float3x3 ACESInputMat =
		{
			{ 0.59719, 0.35458, 0.04823 },
			{ 0.07600, 0.90834, 0.01566 },
			{ 0.02840, 0.13383, 0.83777 }
		};

		// ODT_SAT => XYZ => D60_2_D65 => sRGB
		static const float3x3 ACESOutputMat =
		{
			{ 1.60475, -0.53108, -0.07367 },
			{ -0.10208, 1.10813, -0.00605 },
			{ -0.00327, -0.07276, 1.07602 }
		};
		
		Color = mul(ACESInputMat, Color);
		
		float3 a = Color * (Color + 0.0245786f) - 0.000090537f;
		float3 b = Color * (0.983729f * Color + 0.4329510f) + 0.238081f;
		Color = a / b;
		
		Color = mul(ACESOutputMat, Color);
		
		return saturate(Color);
	}

	float3 TonemapHable(float3 Color)
	{
		const float A = 0.15f;
		const float B = 0.50f;
		const float C = 0.10f;
		const float D = 0.20f;
		const float E = 0.02f;
		const float F = 0.30f;
		const float W = 11.2f;
		const float exposure = 2.0f;

		Color *= exposure;
		Color = ((Color * (A * Color + C * B) + D * E) / (Color * (A * Color + B) + D * F)) - E / F;
		
		float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
		Color /= white;

		return saturate(Color);
	}
		
	static const float3x3 AgXTransformMatrix = float3x3(
		0.8424010709504686f,	0.04240107095046854f,	0.04240107095046854f,
		0.07843650156180276f,	0.8784365015618028f,	0.07843650156180276f,
		0.0791624274877287f,	0.0791624274877287f,	0.8791624274877287f
	);
	
	static const float3x3 AgXTransformInvMatrix = float3x3(
		 1.1969986613119143f,	-0.053001338688085674f, -0.053001338688085674f,
		-0.09804562695225345f,	 1.1519543730477466f,	-0.09804562695225345f,
		-0.09895303435966087f,	-0.09895303435966087f,	 1.151046965640339f
	);
		
	float3 AgXContrastApproximation(float3 Color)
	{
		float3 x2 = Color * Color;
		float3 x4 = x2 * x2;
		
		return  + 15.5f * x4 * x2
				- 40.14f * x4 * Color
				+ 31.96f * x4
				- 6.868f * x2 * Color
				+ 0.4298f * x2
				+ 0.1191f * Color
				- 0.00232f;
	}
	
	float3 AgX(float3 Color)
	{
		Color = mul(Color, AgXTransformMatrix);
		
		const float minEv = -12.473931188332413f;
		const float maxEv = 4.026068811667588f;

		Color = clamp(log2(Color), minEv, maxEv);
		Color = (Color - minEv) / (maxEv - minEv);
		
		return AgXContrastApproximation(Color);
	}
	
	float3 AgxEotf(float3 Color)
	{
		Color = mul(Color, AgXTransformInvMatrix);
  
		// Currently using sRGB texture format.
		//Color = pow(Color, 2.2f);

		return Color;
	}
	
	float3 TonemapAgX(float3 Color)
	{
		Color = AgX(Color);
	
		float luma = GetLuminance(Color);
  
		float3	offset		= float3(0.0f, 0.0f, 0.0f);
		float3	slope		= float3(1.0f, 1.0f, 1.0f);
		float3	power		= float3(1.0f, 1.0f, 1.0f);
		float	saturation	= 1.0f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);
		
		return saturate(Color);
	}
	
	float3 TonemapAgXPunchy(float3 Color)
	{
		Color = AgX(Color);

		float luma = GetLuminance(Color);
  
		float3	offset		= float3(0.0f, 0.0f, 0.0f);
		float3	slope		= float3(1.0f, 1.0f, 1.0f);
		float3	power		= float3(1.35f, 1.35f, 1.35f);
		float	saturation	= 1.4f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);

		return saturate(Color);
	}
	
	float3 TonemapAgXGolden(float3 Color)
	{
		Color = AgX(Color);

		float luma = GetLuminance(Color);
  
		float3 offset		= float3(0.0f, 0.0f, 0.0f);
		float3 slope		= float3(1.0f, 0.9f, 0.5f);
		float3 power		= float3(0.8f, 0.8f, 0.8f);
		float  saturation	= 0.8f;
  
		Color = pow(Color * slope + offset, power);
		Color = luma + saturation * (Color - luma);
		Color = AgxEotf(Color);

		return saturate(Color);
	}
	
	namespace AMD
	{
		float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
		{
			return
				-((-pow(midIn, contrast) + (midOut * (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) -
			    pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut)) /
			    (pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut)) /
			    (pow(midIn, contrast * shoulder) * midOut));
		}

		// General tonemapping operator, build 'c' term.
		float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
		{
			return (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) - pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut) /
				(pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut);
		}

		// General tonemapping operator, p := {contrast,shoulder,b,c}.
		float ColTone(float x, float4 p)
		{
			float z = pow(x, p.r);
			return z / (pow(z, p.g) * p.b + p.a);
		}
		
		float3 Tonemap(float3 color)
		{
			static float hdrMax = 16.0; // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
			static float contrast = 2.0; // Use as a baseline to tune the amount of contrast the tonemapper has.
			static float shoulder = 1.0; // Likely don’t need to mess with this factor, unless matching existing tonemapper is not working well..
			static float midIn = 0.18; // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
			static float midOut = 0.18; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.

			float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
			float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

#define EPS 1e-6f
			float peak = max(color.r, max(color.g, color.b));
			peak = max(EPS, peak);

			float3 ratio = color / peak;
			peak = ColTone(peak, float4(contrast, shoulder, b, c));
    // then process ratio

    // probably want send these pre-computed (so send over saturation/crossSaturation as a constant)
			float crosstalk = 4.0; // controls amount of channel crosstalk
			float saturation = contrast; // full tonal range saturation control
			float crossSaturation = contrast * 16.0; // crosstalk saturation

			float white = 1.0;

    // wrap crosstalk in transform
			ratio = pow(abs(ratio), saturation / crossSaturation);
			ratio = lerp(ratio, white, pow(peak, crosstalk));
			ratio = pow(abs(ratio), crossSaturation);

    // then apply ratio to peak
			color = peak * ratio;
			return color;
		}
	}

	float3 ApplyTonemapping(float3 Color, float ExposureScale, uint Type)
	{
		const float3 output = Color;
		
		switch (Type)
		{
			case TypeACESSimple:			return TonemapACESSimple(output);
			case TypeACESFilm:				return TonemapACESFilm(output);
			case TypeAMD:					return AMD::Tonemap(output);
			case TypeAgX:					return TonemapAgX(output);
			case TypeAgXPunchy:				return TonemapAgXPunchy(output);
			case TypeAgXGolden:				return TonemapAgXGolden(output);
			case TypeReinhard:				return TonemapReinhard(output * ExposureScale);
			case TypeGammaCorrection:		return TonemapGammaCorrection(output);
			case TypeUncharted2:			return TonemapUncharted2(output);
			case TypeHable:					return TonemapHable(output);
			default:						return TonemapACESSimple(output);
		}
	}
	
} // namespace Tonemapping

#endif // TONEMAPPING_HLSL
