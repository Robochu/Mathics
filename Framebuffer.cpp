module;
#define cimg_display 0
#define cimg_use_tiff
#pragma warning(disable : 4005)
#pragma warning(disable : 5106)
#include "CImg.h"

#include <glad/glad.h>

export module graphics:Framebuffer;

import :Material;

import math;
import color;

import <tuple>;
import <vector>;
import <string>;
import <algorithm>;
import <numbers>;
import <cmath>;
import <cstdint>;
import <cstddef>;

export namespace graphics
{
	struct DirectionalLight;
	class PointLight;

	class Framebuffer
	{
		std::vector<math::Vec4> buffer;
		std::vector<float> zBuffer;
		std::size_t width;
		std::size_t height;

	public:
		Framebuffer() = default;
		explicit Framebuffer(const std::size_t width, const std::size_t height) :
			buffer(width * height), zBuffer(width * height), width(width), height(height) {}
		explicit Framebuffer(const std::string& filename)
		{
			cimg_library::CImg<float> image(filename.c_str());
			image.mirror('y');
			width = image.width();
			height = image.height();
			buffer.resize(width * height);
			for (std::size_t i = 0; i < width * height; i++)
			{
				if (image.spectrum() == 1)
				{
					buffer[i].r() = image.data(0, 0, 0, 0)[i] / 255.0f;
					buffer[i].g() = buffer[i].r();
					buffer[i].b() = buffer[i].r();
					buffer[i].a() = 1.0f;
				}
				else if (image.spectrum() == 3)
				{
					for (unsigned int j = 0; j < 3; j++)
					{
						buffer[i][j] = image.data(0, 0, 0, j)[i] / 255.0f;
					}
					buffer[i].a() = 1.0f;
				}
				else
				{
					for (unsigned int j = 0; j < 4; j++)
					{
						buffer[i][j] = image.data(0, 0, 0, j)[i] / 255.0f;
					}
				}
			}
		}

		math::Vec4* operator[](const std::size_t i)
		{
			return buffer.data() + i * width;
		}
		const math::Vec4* operator[](const std::size_t i) const
		{
			return buffer.data() + i * width;
		}

		float& zLookup(const std::size_t x, const std::size_t y)
		{
			return zBuffer[y * width + x];
		}
		const float& zLookup(const std::size_t x, const std::size_t y) const
		{
			return zBuffer[y * width + x];
		}

		math::Vec4 bilinearLookup(const float x, const float y) const
		{
			const std::size_t x1 = static_cast<std::size_t>(x);
			const std::size_t x2 = x1 + 1;
			const std::size_t y1 = static_cast<std::size_t>(y);
			const std::size_t y2 = y1 + 1;
			if (x2 >= width || y2 >= height)
			{
				return buffer[y1 * width + x1];
			}

			const float fx1 = static_cast<float>(x1);
			const float fx2 = static_cast<float>(x2);
			const float fy1 = static_cast<float>(y1);
			const float fy2 = static_cast<float>(y2);
			return (fx2 - x) * (fy2 - y) * buffer[y1 * width + x1] +
				(x - fx1) * (fy2 - y) * buffer[y1 * width + x2] +
				(fx2 - x) * (y - fy1) * buffer[y2 * width + x1] +
				(x - fx1) * (y - fy1) * buffer[y2 * width + x2];
		}

		float getVisibility(const std::size_t x, const std::size_t y, const float z) const
		{
			static const float epsilon = 0.1f;
			return z >= (zBuffer[y * width + x] - epsilon);
		}
		float getBilinearVisibility(const float x, const float y, const float z) const
		{
			const std::size_t x1 = static_cast<std::size_t>(x);
			const std::size_t x2 = x1 + 1;
			const std::size_t y1 = static_cast<std::size_t>(y);
			const std::size_t y2 = y1 + 1;
			if (x2 >= width || y2 >= height)
			{
				return getVisibility(x1, y1, z);
			}

			const float fx1 = static_cast<float>(x1);
			const float fx2 = static_cast<float>(x2);
			const float fy1 = static_cast<float>(y1);
			const float fy2 = static_cast<float>(y2);
			return (fx2 - x) * (fy2 - y) * getVisibility(x1, y1, z) +
				(x - fx1) * (fy2 - y) * getVisibility(x2, y1, z) +
				(fx2 - x) * (y - fy1) * getVisibility(x1, y2, z) +
				(x - fx1) * (y - fy1) * getVisibility(x2, y2, z);
		}

		void fill(const math::Vec4& color)
		{
			for (std::size_t i = 0; i < width * height; i++)
			{
				buffer[i] = color;
			}
		}
		void clear()
		{
			fill(color::empty);
		}

		void zFill(const float z)
		{
			for (std::size_t i = 0; i < width * height; i++)
			{
				zBuffer[i] = z;
			}
		}
		void zClear()
		{
			zFill(0.0f);
		}

		// Only affect the z-buffer.
		void prerenderTriangle(const math::PinholeCamera& camera,
			const math::Vec3& t1, const math::Vec3& t2, const math::Vec3& t3)
		{
			// https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
			// https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/
			// http://devmaster.net/forums/topic/1145-advanced-rasterization/ (accessible with
			// Wayback Machine)

			// Back-face culling.
			if ((t1 - camera.center).dot((t2 - t1).cross(t3 - t1)) >= 0.0f)
			{
				return;
			}

			const math::Vec3 p1 = camera.project(t1);
			const math::Vec3 p2 = camera.project(t2);
			const math::Vec3 p3 = camera.project(t3);

			if (p1.z() <= 0.0f || p2.z() <= 0.0f || p3.z() <= 0.0f)
			{
				return;
			}

			// 4-bit subpixel precision.
			const int p1x = static_cast<int>(std::round(p1.x() * 16.0f));
			const int p1y = static_cast<int>(std::round(p1.y() * 16.0f));
			const int p2x = static_cast<int>(std::round(p2.x() * 16.0f));
			const int p2y = static_cast<int>(std::round(p2.y() * 16.0f));
			const int p3x = static_cast<int>(std::round(p3.x() * 16.0f));
			const int p3y = static_cast<int>(std::round(p3.y() * 16.0f));

			const int minX = std::max((std::min(std::min(p1x, p2x), p3x) + 15) >> 4, 0);
			const int minY = std::max((std::min(std::min(p1y, p2y), p3y) + 15) >> 4, 0);
			const int maxX = std::min(
				(std::max(std::max(p1x, p2x), p3x) + 15) >> 4,
				static_cast<int>(width) - 1
			);
			const int maxY = std::min(
				(std::max(std::max(p1y, p2y), p3y) + 15) >> 4,
				static_cast<int>(height) - 1
			);

			const int a1 = p1y - p2y;
			const int a2 = p2y - p3y;
			const int a3 = p3y - p1y;
			const int b1 = p2x - p1x;
			const int b2 = p3x - p2x;
			const int b3 = p1x - p3x;

			const int fa1 = a1 << 4;
			const int fa2 = a2 << 4;
			const int fa3 = a3 << 4;
			const int fb1 = b1 << 4;
			const int fb2 = b2 << 4;
			const int fb3 = b3 << 4;

			int u1 = b2 * ((minY << 4) - p2y) + a2 * ((minX << 4) - p2x);
			int u2 = b3 * ((minY << 4) - p3y) + a3 * ((minX << 4) - p3x);
			int u3 = b1 * ((minY << 4) - p1y) + a1 * ((minX << 4) - p1x);

			const math::Vec3 rc = (math::Mat3(
				p1.x(), p1.y(), 1.0f,
				p2.x(), p2.y(), 1.0f,
				p3.x(), p3.y(), 1.0f
			).inverse() * math::Vec3(p1.z(), p2.z(), p3.z()));
			float w = rc.dot({static_cast<float>(minX), static_cast<float>(minY), 1.0f});

			float* zb = zBuffer.data() + minY * width;
			for (int y = minY; y <= maxY; y++)
			{
				int v1 = u1;
				int v2 = u2;
				int v3 = u3;
				float z = w;
				for (int x = minX; x <= maxX; x++)
				{
					if ((v1 | v2 | v3) >= 0)
					{
						zb[x] = std::max(zb[x], z);
					}

					v1 += fa2;
					v2 += fa3;
					v3 += fa1;
					z += rc[0];
				}

				u1 += fb2;
				u2 += fb3;
				u3 += fb1;
				w += rc[1];
				zb += width;
			}
		}
		void renderTriangle(const math::PinholeCamera& camera,
			const math::Vec3& t1, const math::Vec3& t2, const math::Vec3& t3,
			const math::Vec4& c1, const math::Vec4& c2, const math::Vec4& c3,
			const math::Vec3& n1, const math::Vec3& n2, const math::Vec3& n3,
			std::vector<DirectionalLight>& directionalLights, std::vector<PointLight>& pointLights,
			const Material& material);
		void renderTriangle(const math::PinholeCamera& camera, const Framebuffer& texture,
			const math::Vec3& t1, const math::Vec3& t2, const math::Vec3& t3,
			const math::Vec2& r1, const math::Vec2& r2, const math::Vec2& r3,
			const math::Vec3& n1, const math::Vec3& n2, const math::Vec3& n3,
			std::vector<DirectionalLight>& directionalLights, std::vector<PointLight>& pointLights,
			const Material& material);

		void blit() const
		{
			glDrawPixels(static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA,
				GL_FLOAT, buffer.data());
		}
		void blit(Framebuffer& surface, const int offsetX = 0, const int offsetY = 0) const
		{
			const std::size_t minX = std::clamp(offsetX, 0, static_cast<int>(surface.width));
			const std::size_t minY = std::clamp(offsetY, 0, static_cast<int>(surface.height));
			const std::size_t maxX = std::clamp(offsetX + static_cast<int>(width), 0,
				static_cast<int>(surface.width));
			const std::size_t maxY = std::clamp(offsetY + static_cast<int>(height), 0,
				static_cast<int>(surface.height));
			for (std::size_t x = minX; x < maxX; x++)
			{
				for (std::size_t y = minY; y < maxY; y++)
				{
					surface[y][x] = buffer[(y - offsetY) * width + x - offsetX];
				}
			}
		}

		// Change the color buffer to show the z-buffer in grayscale.
		void zTransform()
		{
			static const float brightnessOffset = 0.5f;
			for (std::size_t i = 0; i < width * height; i++)
			{
				buffer[i] = math::Vec4(-std::exp(-zBuffer[i] * brightnessOffset) + 1.0f);
			}
		}

		Framebuffer flip() const
		{
			Framebuffer result = Framebuffer(width, height);
			for (std::size_t x = 0; x < width; x++)
			{
				for (std::size_t y = 0; y < height; y++)
				{
					result[height - 1 - y][width - 1 - x] = buffer[y * width + x];
				}
			}
			return result;
		}

		std::size_t getWidth() const
		{
			return width;
		}
		std::size_t getHeight() const
		{
			return height;
		}

		void saveTIFF(const std::string& filename)
		{
			cimg_library::CImg<float> image(static_cast<unsigned int>(width),
				static_cast<unsigned int>(height), 1, 4, true);
			for (std::size_t i = 0; i < width * height; i++)
			{
				for (unsigned int j = 0; j < 4; j++)
				{
					*(image.data(0, 0, 0, j) + i) = buffer[i][j];
				}
			}
			image.mirror('y').save_tiff(filename.c_str());
		}
	};
}
