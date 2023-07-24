export module math:PinholeCamera;

import :forward;
import :Vector;
import :Matrix;

import <vector>;
import <cmath>;

export namespace math
{
	class PinholeCamera
	{
		constexpr Mat3 getProjectionMatrix() const
		{
			return Mat3({{a, b, c}}).transpose().inverse();
		}

	public:
		Vec3 center;
		Vec3 a;
		Vec3 b;
		Vec3 c;
		Mat3 projectionMatrix;
		unsigned int width;
		unsigned int height;

		// Shadow map rasterization parameters.
		Mat3 sc;
		Vec3 sp;
		Vec3 p;

		constexpr PinholeCamera() = default;
		explicit constexpr PinholeCamera(const unsigned int width, const unsigned int height,
			const float hfov) : center(0.0f), a(1.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f),
			c(-(width / 2.0f), -(height / 2.0f), -(width / (2.0f * std::tan(hfov / 2.0f)))),
			projectionMatrix(getProjectionMatrix()), width(width), height(height) {}
		explicit constexpr PinholeCamera(const unsigned int width, const unsigned int height,
			const float hfov, const Vec3& center, const Vec3& direction, const Vec3& up) :
			PinholeCamera(width, height, hfov)
		{
			this->center = center;
			orient(direction, up);
		}

		constexpr Vec3 getViewDirection() const
		{
			return b.cross(a);
		}
		constexpr float getFocalLength() const
		{
			return c.dot(getViewDirection());
		}
		float getHorizontalFieldOfView() const
		{
			return 2.0f * std::atan(width / 2.0f / getFocalLength());
		}
		float getVerticalFieldOfView() const
		{
			return 2.0f * std::atan(height / 2.0f / getFocalLength());
		}
		constexpr Vec2 getPrincipalPoint() const
		{
			return {-c.dot(a), -c.dot(b)};
		}

		constexpr void translateHorizontally(const float distance)
		{
			center += a * distance;
		}
		constexpr void translateVertically(const float distance)
		{
			center += b * distance;
		}
		constexpr void translateDepth(const float distance)
		{
			center += getViewDirection() * distance;
		}

		void pan(const float theta) // TODO constexpr
		{
			a.rotateAboutAxis(b, theta);
			c.rotateAboutAxis(b, theta);
			projectionMatrix = getProjectionMatrix();
		}
		void tilt(const float theta)
		{
			b.rotateAboutAxis(a, theta);
			c.rotateAboutAxis(a, theta);
			projectionMatrix = getProjectionMatrix();
		}
		void roll(const float theta)
		{
			const Vec3 viewDirection = getViewDirection();
			a.rotateAboutAxis(viewDirection, theta);
			b.rotateAboutAxis(viewDirection, theta);
			c.rotateAboutAxis(viewDirection, theta);
			projectionMatrix = getProjectionMatrix();
		}

		constexpr void zoom(const float multiplier)
		{
			c = getViewDirection() * getFocalLength() * multiplier - a * (width / 2.0f) -
				b * (height / 2.0f);
			projectionMatrix = getProjectionMatrix();
		}

		constexpr void orient(Vec3 direction, const Vec3& up)
		{
			const float focalLength = getFocalLength();
			direction.normalize();
			a = direction.cross(up.unit());
			b = a.cross(direction);
			c = direction * focalLength - a * (width / 2.0f) - b * (height / 2.0f);
			projectionMatrix = getProjectionMatrix();
		}
		constexpr void point(const Vec3& direction)
		{
			orient(direction, b);
		}
		constexpr void setUp(const Vec3& up)
		{
			orient(getViewDirection(), up);
		}
		constexpr void lookAtAndUp(const Vec3& object, const Vec3& up)
		{
			orient(object - center, up);
		}
		constexpr void lookAt(const Vec3& object)
		{
			point(object - center);
		}

		constexpr void resize(unsigned int width, unsigned int height)
		{
			this->width = width;
			this->height = height;
			orient(getViewDirection(), b);
		}

		constexpr Vec3 project(const Vec3& point) const
		{
			Vec3 q = projectionMatrix * (point - center);
			return {q.x() / q.z(), q.y() / q.z(), 1.0f / q.z()};
		}
		constexpr Vec3 unproject(const Vec3& projection) const
		{
			return center + (a * projection.x() + b * projection.y() + c) / projection.z();
		}
	};

	std::vector<PinholeCamera> interpolate(const PinholeCamera& c1, const PinholeCamera& c2,
		const std::size_t steps)
	{
		std::vector<PinholeCamera> cameras(steps);
		PinholeCamera camera = c1;
		for (std::size_t step = 0; step < steps; step++)
		{
			cameras[step] = camera;
			camera = c1; // Necessary to reset the camera for numerical stability.
			camera.orient(
				c1.getViewDirection() + (c2.getViewDirection() - c1.getViewDirection()) *
					static_cast<float>(step + 1) / static_cast<float>(steps - 1),
				c1.b + (c2.b - c1.b) * static_cast<float>(step + 1) / static_cast<float>(steps - 1)
			);
			camera.center = c1.center + (c2.center - c1.center) * static_cast<float>(step + 1) /
				static_cast<float>(steps - 1);
		}
		return cameras;
	}
}
