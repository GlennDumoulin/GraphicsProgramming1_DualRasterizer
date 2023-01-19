#pragma once
#include <cmath>

namespace dae
{
	/* --- HELPER STRUCTS --- */
	struct Int2
	{
		int x{};
		int y{};
	};

	/* --- CONSTANTS --- */
	constexpr auto PI = 3.14159265358979323846f;
	constexpr auto PI_DIV_2 = 1.57079632679489661923f;
	constexpr auto PI_DIV_4 = 0.785398163397448309616f;
	constexpr auto PI_2 = 6.283185307179586476925f;
	constexpr auto PI_4 = 12.56637061435917295385f;

	constexpr auto PI_INV = 1.f / PI;

	constexpr auto TO_DEGREES = (180.0f / PI);
	constexpr auto TO_RADIANS = (PI / 180.0f);

	/* --- HELPER FUNCTIONS --- */
	inline float Square(const float a)
	{
		return a * a;
	}

	inline float Lerpf(const float a, const float b, const float factor)
	{
		return ((1 - factor) * a) + (factor * b);
	}

	inline bool AreEqual(const float a, const float b, const float epsilon = FLT_EPSILON)
	{
		return abs(a - b) < epsilon;
	}

	inline int Clamp(const int v, const int min, const int max)
	{
		if (v < min) return min;
		if (v > max) return max;
		return v;
	}

	inline float Clamp(const float v, const float min, const float max)
	{
		if (v < min) return min;
		if (v > max) return max;
		return v;
	}

	inline float Remap(const float value, const float min, const float max)
	{
		const float clampedValue{ std::clamp(value, min, max) };

		return (clampedValue - min) / (max - min);
	}

	inline float Saturate(const float v)
	{
		if (v < 0.f) return 0.f;
		if (v > 1.f) return 1.f;
		return v;
	}

	inline int Sign(const int v)
	{
		if (v < 0) return -1;
		if (v > 0) return 1;
		return 0;
	}
}
