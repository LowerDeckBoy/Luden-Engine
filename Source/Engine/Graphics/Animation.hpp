#pragma once

namespace Luden
{
	//enum class EAnimationChannel
	struct FAnimationChannel
	{
		enum class EPath
		{
			Translation,
			Rotation,
			Scale
		};

		enum class EInterpolation
		{
			Linear,
			Step,
			Cubic
		};
	};

	enum class EAnimationSampler
	{

	};

	// 
	//struct FAnimation
	//{
	//	std::string Name;
	//
	//	FAnimationChannel Channel;
	//	EAnimationSampler Sampler;
	//
	//	uint32 StartTime;
	//	uint32 EndTime;
	//	uint32 Length;
	//};
} // namespace Luden
