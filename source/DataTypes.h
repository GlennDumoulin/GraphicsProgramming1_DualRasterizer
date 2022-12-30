#pragma once
#include "Math.h"

namespace dae
{
	//Enums
	enum class PrimitiveTopology
	{
		TRIANGLE_LIST,
		TRIANGLE_STRIP
	};

	enum class CullMode
	{
		BACK,
		FRONT,
		NONE,
	};

	enum class ShadingMode
	{
		OBSERVED_AREA,
		DIFFUSE,
		SPECULAR,
		COMBINED,
	};

	enum class SamplerState
	{
		POINT,
		LINEAR,
		ANISOTROPIC,
	};

	enum class EffectType
	{
		STANDARD,
		TRANSPARENCY,
	};

	//Structs
	struct Vertex
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct VertexOut
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};
}
