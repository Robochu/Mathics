import math;
import graphics;

#include <vector>
#include <cmath>

namespace graphics
{
	// In a separate file to avoid a cyclic dependency.
	void Framebuffer::renderTriangle(const math::PinholeCamera& camera,
		const math::Vec3& t1, const math::Vec3& t2, const math::Vec3& t3,
		const math::Vec4& c1, const math::Vec4& c2, const math::Vec4& c3,
		const math::Vec3& n1, const math::Vec3& n2, const math::Vec3& n3,
		std::vector<DirectionalLight>& directionalLights, std::vector<PointLight>& pointLights,
		const Material& material)
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

		int u1 = (p3x - p2x) * ((minY << 4) - p2y) - (p3y - p2y) * ((minX << 4) - p2x);
		int u2 = (p1x - p3x) * ((minY << 4) - p3y) - (p1y - p3y) * ((minX << 4) - p3x);
		int u3 = (p2x - p1x) * ((minY << 4) - p1y) - (p2y - p1y) * ((minX << 4) - p1x);

		const math::Vec3 lv = math::Vec3(static_cast<float>(minX), static_cast<float>(minY), 1.0f);
		const math::Matrix<3, 8> rc = (math::Mat3(
			p1.x(), p1.y(), 1.0f,
			p2.x(), p2.y(), 1.0f,
			p3.x(), p3.y(), 1.0f
		).inverse() * math::Matrix<3, 8>(
			p1.z(), c1.r(), c1.g(), c1.b(), c1.a(), n1.x(), n1.y(), n1.z(),
			p2.z(), c2.r(), c2.g(), c2.b(), c2.a(), n2.x(), n2.y(), n2.z(),
			p3.z(), c3.r(), c3.g(), c3.b(), c3.a(), n3.x(), n3.y(), n3.z()
		));
		math::Vector<8> q = lv * rc;

		const math::Mat3 cm1 = math::Mat3(camera.a, camera.b, camera.c).transpose();
		for (PointLight& pointLight : pointLights)
		{
			for (std::size_t i = 0; i < 3; i++)
			{
				const math::Vec3 sf = pointLight.shadowMap.cameras[i].projectionMatrix *
					(camera.center - pointLight.shadowMap.cameras[i].center);
				pointLight.shadowMap.cameras[i].sc =
					pointLight.shadowMap.cameras[i].projectionMatrix * cm1;
				pointLight.shadowMap.cameras[i].sp = q[0] * sf +
					pointLight.shadowMap.cameras[i].sc * lv;
				pointLight.shadowMap.cameras[i].sc[0][0] += rc[0][0] * sf[0];
				pointLight.shadowMap.cameras[i].sc[1][0] += rc[0][0] * sf[1];
				pointLight.shadowMap.cameras[i].sc[2][0] += rc[0][0] * sf[2];
				pointLight.shadowMap.cameras[i].sc[0][1] += rc[1][0] * sf[0];
				pointLight.shadowMap.cameras[i].sc[1][1] += rc[1][0] * sf[1];
				pointLight.shadowMap.cameras[i].sc[2][1] += rc[1][0] * sf[2];
			}
		}

		math::Vec4* cb = buffer.data() + minY * width;
		float* zb = zBuffer.data() + minY * width;
		for (int y = minY; y <= maxY; y++)
		{
			int v1 = u1;
			int v2 = u2;
			int v3 = u3;
			math::Vector<8> p = q;
			for (PointLight& pointLight : pointLights)
			{
				for (std::size_t i = 0; i < 3; i++)
				{
					pointLight.shadowMap.cameras[i].p = pointLight.shadowMap.cameras[i].sp;
				}
			}

			for (int x = minX; x <= maxX; x++)
			{
				if ((v1 | v2 | v3) >= 0 && p[0] >= zb[x])
				{
					const math::Vec4 color = light(
						p.subvector<1, 5>(), p.subvector<5, 8>().unit(),
						camera.unproject({static_cast<float>(x), static_cast<float>(y), p[0]}),
						camera.center, directionalLights, pointLights, p[0], material
					);
					cb[x] = color + (1.0f - color.a()) * cb[x];
				}

				v1 += fa2;
				v2 += fa3;
				v3 += fa1;
				p += rc[0];
				for (PointLight& pointLight : pointLights)
				{
					for (std::size_t i = 0; i < 3; i++)
					{
						pointLight.shadowMap.cameras[i].p +=
							pointLight.shadowMap.cameras[i].sc.getColumn(0);
					}
				}
			}

			u1 += fb2;
			u2 += fb3;
			u3 += fb1;
			q += rc[1];
			for (PointLight& pointLight : pointLights)
			{
				for (std::size_t i = 0; i < 3; i++)
				{
					pointLight.shadowMap.cameras[i].sp +=
						pointLight.shadowMap.cameras[i].sc.getColumn(1);
				}
			}
			cb += width;
			zb += width;
		}
	}

	void Framebuffer::renderTriangle(const math::PinholeCamera& camera, const Framebuffer& texture,
		const math::Vec3& t1, const math::Vec3& t2, const math::Vec3& t3,
		const math::Vec2& r1, const math::Vec2& r2, const math::Vec2& r3,
		const math::Vec3& n1, const math::Vec3& n2, const math::Vec3& n3,
		std::vector<DirectionalLight>& directionalLights, std::vector<PointLight>& pointLights,
		const Material& material)
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

		int u1 = (p3x - p2x) * ((minY << 4) - p2y) - (p3y - p2y) * ((minX << 4) - p2x);
		int u2 = (p1x - p3x) * ((minY << 4) - p3y) - (p1y - p3y) * ((minX << 4) - p3x);
		int u3 = (p2x - p1x) * ((minY << 4) - p1y) - (p2y - p1y) * ((minX << 4) - p1x);

		const math::Vec3 lv = math::Vec3(static_cast<float>(minX), static_cast<float>(minY), 1.0f);
		const math::Matrix<3, 4> rc = (math::Mat3(
			p1.x(), p1.y(), 1.0f,
			p2.x(), p2.y(), 1.0f,
			p3.x(), p3.y(), 1.0f
		).inverse() * math::Matrix<3, 4>(
			p1.z(), n1.x(), n1.y(), n1.z(),
			p2.z(), n2.x(), n2.y(), n2.z(),
			p3.z(), n3.x(), n3.y(), n3.z()
		));
		math::Vector<4> q = lv * rc;

		const math::Mat3 cm = math::Mat3(camera.a, camera.b, camera.c);
		const math::Mat3 cm1 = cm.transpose();
		for (PointLight& pointLight : pointLights)
		{
			for (std::size_t i = 0; i < 3; i++)
			{
				const math::Vec3 sf = pointLight.shadowMap.cameras[i].projectionMatrix *
					(camera.center - pointLight.shadowMap.cameras[i].center);
				pointLight.shadowMap.cameras[i].sc =
					pointLight.shadowMap.cameras[i].projectionMatrix * cm1;
				pointLight.shadowMap.cameras[i].sp = q[0] * sf +
					pointLight.shadowMap.cameras[i].sc * lv;
				pointLight.shadowMap.cameras[i].sc[0][0] += rc[0][0] * sf[0];
				pointLight.shadowMap.cameras[i].sc[1][0] += rc[0][0] * sf[1];
				pointLight.shadowMap.cameras[i].sc[2][0] += rc[0][0] * sf[2];
				pointLight.shadowMap.cameras[i].sc[0][1] += rc[1][0] * sf[0];
				pointLight.shadowMap.cameras[i].sc[1][1] += rc[1][0] * sf[1];
				pointLight.shadowMap.cameras[i].sc[2][1] += rc[1][0] * sf[2];
			}
		}

		const math::Mat3 tc = cm * math::Mat3(t1 - camera.center, t2 - camera.center, t3 -
			camera.center).inverse();
		const math::Matrix<3, 2> dc = tc * math::Matrix<3, 2>(r1, r2, r3);
		const math::Vec3 nc = {tc[0].sum(), tc[1].sum(), tc[2].sum()};
		float rdx = dc.getColumn(0).dot(lv);
		float rdy = dc.getColumn(1).dot(lv);
		float rn = nc.dot(lv);

		math::Vec4* cb = buffer.data() + minY * width;
		float* zb = zBuffer.data() + minY * width;
		for (int y = minY; y <= maxY; y++)
		{
			int v1 = u1;
			int v2 = u2;
			int v3 = u3;
			math::Vector<4> p = q;
			float dx = rdx;
			float dy = rdy;
			float n = rn;
			for (PointLight& pointLight : pointLights)
			{
				for (std::size_t i = 0; i < 3; i++)
				{
					pointLight.shadowMap.cameras[i].p = pointLight.shadowMap.cameras[i].sp;
				}
			}

			for (int x = minX; x <= maxX; x++)
			{
				if ((v1 | v2 | v3) >= 0 && p[0] >= zb[x])
				{
					float tx = dx / n;
					const float fx = std::floor(tx);
					float ty = dy / n;
					const float fy = std::floor(ty);
					tx = static_cast<int>(fx) % 2 ? 1.0f + fx - tx : tx - fx;
					ty = static_cast<int>(fy) % 2 ? 1.0f + fy - ty : ty - fy;
					tx *= static_cast<float>(texture.getWidth() - 1);
					ty *= static_cast<float>(texture.getHeight() - 1);
					const math::Vec4 color = light(
						texture.bilinearLookup(tx, ty), p.subvector<1, 4>().unit(),
						camera.unproject({static_cast<float>(x), static_cast<float>(y), p[0]}),
						camera.center, directionalLights, pointLights, p[0], material
					);
					cb[x] = color + (1.0f - color.a()) * cb[x];
				}

				v1 += fa2;
				v2 += fa3;
				v3 += fa1;
				p += rc[0];
				dx += dc[0][0];
				dy += dc[0][1];
				n += nc[0];
				for (PointLight& pointLight : pointLights)
				{
					for (std::size_t i = 0; i < 3; i++)
					{
						pointLight.shadowMap.cameras[i].p +=
							pointLight.shadowMap.cameras[i].sc.getColumn(0);
					}
				}
			}

			u1 += fb2;
			u2 += fb3;
			u3 += fb1;
			q += rc[1];
			rdx += dc[1][0];
			rdy += dc[1][1];
			rn += nc[1];
			for (PointLight& pointLight : pointLights)
			{
				for (std::size_t i = 0; i < 3; i++)
				{
					pointLight.shadowMap.cameras[i].sp +=
						pointLight.shadowMap.cameras[i].sc.getColumn(1);
				}
			}
			cb += width;
			zb += width;
		}
	}
}
