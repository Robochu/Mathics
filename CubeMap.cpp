export module graphics:CubeMap;

import :Framebuffer;
import :TriangleMesh;

import math;
import color;

import <array>;
import <algorithm>;
import <numbers>;

export namespace graphics
{
	struct CubeMap
	{
		std::array<math::PinholeCamera, 6> cameras;
		std::array<Framebuffer, 6> framebuffers;

		std::size_t previousHit;

		void unrollCameras()
		{
			cameras[1].tilt(std::numbers::pi_v<float> * 0.5f);
			cameras[2].pan(std::numbers::pi_v<float> * 0.5f);
			cameras[3].tilt(std::numbers::pi_v<float>);
			cameras[3].roll(std::numbers::pi_v<float> * 1.5f);
			cameras[4].tilt(std::numbers::pi_v<float> * 1.5f);
			cameras[4].roll(std::numbers::pi_v<float> * 1.5f);
			cameras[5].pan(std::numbers::pi_v<float> * 1.5f);
			cameras[5].roll(std::numbers::pi_v<float> * 0.5f);
		}

		CubeMap() = default;
		explicit CubeMap(const unsigned int resolution, const math::Vec3& position) :
			previousHit(0)
		{
			std::fill(cameras.begin(), cameras.end(), math::PinholeCamera(
				resolution, resolution,
				std::numbers::pi_v<float> / 2.0f, position,
				{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
			));
			unrollCameras();
			std::fill(framebuffers.begin(), framebuffers.end(), Framebuffer(resolution,
				resolution));
		}
		explicit CubeMap(const std::array<Framebuffer, 6>& framebuffers) :
			framebuffers(framebuffers), previousHit(0)
		{
			std::fill(cameras.begin(), cameras.end(), math::PinholeCamera(
				static_cast<unsigned int>(framebuffers[0].getWidth()),
				static_cast<unsigned int>(framebuffers[0].getWidth()),
				std::numbers::pi_v<float> / 2.0f, math::Vec3(0.0f),
				{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
			));
			unrollCameras();
		}

		math::Vec3 getPosition() const
		{
			return cameras[0].center;
		}
		void setPosition(const math::Vec3& position)
		{
			for (math::PinholeCamera& camera : cameras)
			{
				camera.center = position;
			}
		}

		unsigned int getResolution() const
		{
			return cameras[0].width;
		}

		void fill(const math::Vec4& color)
		{
			for (Framebuffer& framebuffer : framebuffers)
			{
				framebuffer.fill(color);
			}
		}
		void clear()
		{
			fill(color::black);
		}

		void zFill(const float z)
		{
			for (Framebuffer& framebuffer : framebuffers)
			{
				framebuffer.zFill(z);
			}
		}
		void zClear()
		{
			zFill(0.0f);
		}

		void prerender(const TriangleMesh& mesh)
		{
			for (std::size_t i = 0; i < 6; i++)
			{
				mesh.prerender(framebuffers[i], cameras[i]);
			}
		}
		void render(const TriangleMesh& mesh, std::vector<DirectionalLight>& directionalLights,
			std::vector<PointLight>& pointLights)
		{
			for (std::size_t i = 0; i < 6; i++)
			{
				mesh.render(framebuffers[i], cameras[i], directionalLights, pointLights);
			}
		}

		void renderOnto(Framebuffer& framebuffer, const math::PinholeCamera& camera)
		{
			// TODO get rid of projection.
			math::Vec3 p = camera.c;
			math::Vec4* cb = &framebuffer[0][0];
			for (std::size_t y = 0; y < framebuffer.getHeight(); y++)
			{
				math::Vec3 r = p;
				for (std::size_t x = 0; x < framebuffer.getWidth(); x++)
				{
					cb[x] = lookup(r);
					r += camera.a;
				}
				p += camera.b;
				cb += framebuffer.getWidth();
			}
		}
		void renderOnto(CubeMap& cubeMap)
		{
			for (std::size_t i = 0; i < 6; i++)
			{
				renderOnto(cubeMap.framebuffers[i], cubeMap.cameras[i]);
			}
		}

		math::Vec4 lookup(math::Vec3 ray)
		{
			ray += getPosition();
			math::Vec3 projection = cameras[previousHit].project(ray);
			if (projection.x() >= 0.0f && projection.x() < cameras[previousHit].width &&
				projection.y() >= 0.0f && projection.y() < cameras[previousHit].width)
			{
				if (projection.z() > 0.0f)
				{
					return framebuffers[previousHit].bilinearLookup(projection.x(),
						projection.y());
				}
				else
				{
					return framebuffers[previousHit + 3].bilinearLookup(projection.y(),
						projection.x());
				}
			}
			for (std::size_t i = 0; i < 3; i++)
			{
				if (i != previousHit)
				{
					projection = cameras[i].project(ray);
					if (projection.x() >= 0.0f && projection.x() < cameras[i].width &&
						projection.y() >= 0.0f && projection.y() < cameras[i].width)
					{
						previousHit = i;
						if (projection.z() > 0.0f)
						{
							return framebuffers[i].bilinearLookup(projection.x(),
								projection.y());
						}
						else
						{
							return framebuffers[i + 3].bilinearLookup(projection.y(),
								projection.x());
						}
					}
				}
			}
			return color::black;
		}

		float getVisibility(const float w)
		{
			math::Vec3 projection = math::Vec3(cameras[previousHit].p[0],
				cameras[previousHit].p[1], w) / cameras[previousHit].p[2];
			if (projection.x() >= 0.0f && projection.x() < cameras[previousHit].width &&
				projection.y() >= 0.0f && projection.y() < cameras[previousHit].width)
			{
				if (projection.z() > 0.0f)
				{
					return framebuffers[previousHit].getBilinearVisibility(projection.x(),
						projection.y(), projection.z());
				}
				else
				{
					return framebuffers[previousHit + 3].getBilinearVisibility(projection.y(),
						projection.x(), -projection.z());
				}
			}
			for (std::size_t i = 0; i < 3; i++)
			{
				if (i != previousHit)
				{
					projection = math::Vec3(cameras[i].p[0], cameras[i].p[1], w) / cameras[i].p[2];
					if (projection.x() >= 0.0f && projection.x() < cameras[i].width &&
						projection.y() >= 0.0f && projection.y() < cameras[i].width)
					{
						previousHit = i;
						if (projection.z() > 0.0f)
						{
							return framebuffers[i].getBilinearVisibility(projection.x(),
								projection.y(), projection.z());
						}
						else
						{
							return framebuffers[i + 3].getBilinearVisibility(projection.y(),
								projection.x(), -projection.z());
						}
					}
				}
			}
			return 0.0f;
		}
	};
}
