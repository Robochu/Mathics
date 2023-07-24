export module math;

export import :forward;
export import :Vector;
export import :Matrix;
export import :PinholeCamera;

import <numbers>;

export namespace math
{
	constexpr float toRadians(const float angle)
	{
		return angle / 180.0f * std::numbers::pi_v<float>;
	}
	constexpr float toDegrees(const float angle)
	{
		return angle * 180.0f / std::numbers::pi_v<float>;
	}

	constexpr bool lineIntersection(const Vec2& l1p1, const Vec2& l1p2, const Vec2& l2p1,
		const Vec2& l2p2, Vec2& intersection)
	{
		const math::Vec2 r = l1p2 - l1p1;
		const math::Vec2 s = l2p2 - l2p1;

		// Do not handle the collinear intersecting case.
		if (r.cross(s) == 0.0f)
		{
			return false;
		}

		const float t = (l2p1 - l1p1).cross(s) / r.cross(s);
		const float u = (l2p1 - l1p1).cross(r) / r.cross(s);

		if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f)
		{
			intersection = l1p1 + t * r;
			return true;
		}

		return false;
	}

	constexpr float power(float base, unsigned int exponent)
	{
		float result = 1.0f;
		while (exponent)
		{
			if (exponent & 1)
			{
				result *= base;
			}
			exponent >>= 1;
			base *= base;
		}
		return result;
	}
}
