export module graphics:TriangleMesh;

import :Framebuffer;
import :Material;

import math;

import <array>;
import <vector>;
import <numbers>;
import <fstream>;
import <stdexcept>;

export namespace graphics
{
	struct DirectionalLight;
	class PointLight;

	struct TriangleMesh
	{
		std::vector<math::Vec3> vertices;
		std::vector<math::Vec4> colors;
		std::vector<std::array<unsigned int, 3>> triangles;
		std::vector<math::Vec3> normals;
		std::vector<math::Vec2> textureCoordinates;
		Framebuffer* texture;
		Material material;

		TriangleMesh(const Material& material = defaultMaterial) :
			texture(nullptr), material(material) {};
		explicit TriangleMesh(const std::string& filename,
			const Material& material = defaultMaterial) : texture(nullptr), material(material)
		{
			addBin(filename);
		}
		explicit TriangleMesh(const std::string& filename, Framebuffer* texture,
			const Material& material = defaultMaterial) : texture(texture), material(material)
		{
			addBin(filename);
		}

		void addTriangle(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec2& r1, const math::Vec2& r2, const math::Vec2& r3)
		{
			const unsigned int i = static_cast<unsigned int>(vertices.size());
			vertices.push_back(p1);
			vertices.push_back(p2);
			vertices.push_back(p3);
			triangles.push_back({i, i + 1, i + 2});
			normals.insert(normals.end(), 3, (p2 - p1).cross(p3 - p1).unit());
			textureCoordinates.push_back(r1);
			textureCoordinates.push_back(r2);
			textureCoordinates.push_back(r3);
		}
		void addTriangle(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec4& c1, const math::Vec4& c2, const math::Vec4& c3)
		{
			const unsigned int i = static_cast<unsigned int>(vertices.size());
			vertices.push_back(p1);
			vertices.push_back(p2);
			vertices.push_back(p3);
			triangles.push_back({i, i + 1, i + 2});
			normals.insert(normals.end(), 3, (p2 - p1).cross(p3 - p1).unit());
			colors.push_back(c1);
			colors.push_back(c2);
			colors.push_back(c3);
		}
		void addTriangle(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec4& color)
		{
			addTriangle(p1, p2, p3, color, color, color);
		}
		void addQuad(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec3& p4, const math::Vec2& r1, const math::Vec2& r2, const math::Vec2& r3,
			const math::Vec2 r4)
		{
			addTriangle(p1, p2, p3, r1, r2, r3);
			addTriangle(p1, p3, p4, r1, r3, r4);
		}
		void addQuad(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec3& p4, const math::Vec4& c1, const math::Vec4& c2, const math::Vec4& c3,
			const math::Vec4 c4)
		{
			addTriangle(p1, p2, p3, c1, c2, c3);
			addTriangle(p1, p3, p4, c1, c3, c4);
		}
		void addQuad(const math::Vec3& p1, const math::Vec3& p2, const math::Vec3& p3,
			const math::Vec3& p4, const math::Vec4& color)
		{
			addQuad(p1, p2, p3, p4, color, color, color, color);
		}

		void addAlignedBox(const math::Vec3& p1, const math::Vec3& p2, const math::Vec4& color)
		{
			const unsigned int j = static_cast<unsigned int>(vertices.size());
			vertices.push_back(p1);
			vertices.push_back({p1.x(), p1.y(), p2.z()});
			vertices.push_back({p1.x(), p2.y(), p1.z()});
			vertices.push_back({p1.x(), p2.y(), p2.z()});
			vertices.push_back({p2.x(), p1.y(), p1.z()});
			vertices.push_back({p2.x(), p1.y(), p2.z()});
			vertices.push_back({p2.x(), p2.y(), p1.z()});
			vertices.push_back(p2);
			triangles.push_back({j, j + 1, j + 5});
			triangles.push_back({j, j + 2, j + 3});
			triangles.push_back({j, j + 3, j + 1});
			triangles.push_back({j, j + 4, j + 6});
			triangles.push_back({j, j + 5, j + 4});
			triangles.push_back({j, j + 6, j + 2});
			triangles.push_back({j + 7, j + 1, j + 3});
			triangles.push_back({j + 7, j + 2, j + 6});
			triangles.push_back({j + 7, j + 3, j + 2});
			triangles.push_back({j + 7, j + 4, j + 5});
			triangles.push_back({j + 7, j + 5, j + 1});
			triangles.push_back({j + 7, j + 6, j + 4});
			colors.insert(colors.end(), 8, color);
		}

		void addAlignedCylinder(const math::Vec3 bottomCenter, const float radius,
			const float height, const unsigned int subdivisions, const math::Vec4& color)
		{
			const unsigned int j = static_cast<unsigned int>(vertices.size());
			vertices.push_back(bottomCenter);
			vertices.push_back(bottomCenter + math::Vec3(0.0f, height, 0.0f));
			for (std::size_t i = 0; i < subdivisions; i++)
			{
				const unsigned int k = static_cast<unsigned int>(vertices.size());
				vertices.push_back(bottomCenter + radius * math::Vec3(std::cos(
					static_cast<float>(i) / subdivisions * 2.0f * std::numbers::pi_v<float>), 0.0f,
					std::sin(static_cast<float>(i) / subdivisions * 2.0f *
						std::numbers::pi_v<float>)));
				vertices.push_back(bottomCenter + radius * math::Vec3(std::cos(
					static_cast<float>(i + 1) / subdivisions * 2.0f * std::numbers::pi_v<float>),
					0.0f, std::sin(static_cast<float>(i + 1) / subdivisions * 2.0f *
						std::numbers::pi_v<float>)));
				vertices.push_back(bottomCenter + radius * math::Vec3(std::cos(
					static_cast<float>(i) / subdivisions * 2.0f * std::numbers::pi_v<float>), 0.0f,
					std::sin(static_cast<float>(i) / subdivisions * 2.0f *
						std::numbers::pi_v<float>)) + math::Vec3(0.0f, height, 0.0f));
				vertices.push_back(bottomCenter + radius * math::Vec3(std::cos(
					static_cast<float>(i + 1) / subdivisions * 2.0f * std::numbers::pi_v<float>),
					0.0f, std::sin(static_cast<float>(i + 1) / subdivisions * 2.0f *
						std::numbers::pi_v<float>)) + math::Vec3(0.0f, height, 0.0f));
				triangles.push_back({j, k, k + 1});
				triangles.push_back({j + 1, k + 3, k + 2});
				triangles.push_back({k + 1, k, k + 2});
				triangles.push_back({k + 1, k + 2, k + 3});
			}
			colors.insert(colors.end(), subdivisions * 4 + 2, color);
		}

		void addBin(const std::string& filename)
		{
			const unsigned int j = static_cast<unsigned int>(vertices.size());
			std::ifstream file(filename, std::ios::binary);
			if (file.fail())
			{
				throw std::runtime_error("Couldn't open file '" + filename + "' for reading!");
			}
			char ch;

			int vertexCount;
			file.read(reinterpret_cast<char*>(&vertexCount), sizeof(int));
			math::Vec3* vertices = new math::Vec3[vertexCount];

			file.read(&ch, 1);
			if (ch != 'y')
			{
				throw std::runtime_error("XYZ data not found in file '" + filename + "'!");
			}

			file.read(&ch, 1);
			math::Vec3* colors = nullptr;
			if (ch == 'y')
			{
				colors = new math::Vec3[vertexCount];
			}

			file.read(&ch, 1);
			math::Vec3* normals = nullptr;
			if (ch == 'y')
			{
				normals = new math::Vec3[vertexCount];
			}

			file.read(&ch, 1);
			math::Vec2* textureCoordinates = nullptr;
			if (ch == 'y')
			{
				textureCoordinates = new math::Vec2[vertexCount];
			}

			file.read(reinterpret_cast<char*>(vertices), vertexCount * 3 * sizeof(float));
			for (std::size_t i = 0; i < static_cast<std::size_t>(vertexCount); i++)
			{
				this->vertices.push_back(vertices[i]);
			}
			delete vertices;
			if (colors)
			{
				file.read(reinterpret_cast<char*>(colors), vertexCount * 3 * sizeof(float));
				for (std::size_t i = 0; i < static_cast<std::size_t>(vertexCount); i++)
				{
					this->colors.push_back({colors[i].x(), colors[i].y(), colors[i].z(), 1.0f});
				}
				delete colors;
			}
			if (normals)
			{
				file.read(reinterpret_cast<char*>(normals), vertexCount * 3 * sizeof(float));
				this->normals.insert(this->normals.end(), normals, normals + vertexCount);
				delete normals;
			}
			if (textureCoordinates)
			{
				file.read(
					reinterpret_cast<char*>(textureCoordinates),
					vertexCount * 2 * sizeof(float)
				);
				this->textureCoordinates.insert(this->textureCoordinates.end(), textureCoordinates,
					textureCoordinates + vertexCount);
				delete textureCoordinates;
			}

			int triangleCount;
			file.read(reinterpret_cast<char*>(&triangleCount), sizeof(int));
			unsigned int* triangles = new unsigned int[triangleCount * 3];
			file.read(
				reinterpret_cast<char*>(triangles),
				triangleCount * 3 * sizeof(unsigned int)
			);
			for (std::size_t i = 0; i < static_cast<std::size_t>(triangleCount); i++)
			{
				this->triangles.push_back({
					triangles[3 * i] + j,
					triangles[3 * i + 1] + j,
					triangles[3 * i + 2] + j
				});
			}
			delete triangles;
			file.close();
		}

		void prerender(Framebuffer& framebuffer, const math::PinholeCamera& camera) const
		{
			for (const std::array<unsigned int, 3>& triangle : triangles)
			{
				if (texture)
				{
					framebuffer.prerenderTriangle(camera, vertices[triangle[0]],
						vertices[triangle[1]], vertices[triangle[2]]);
				}
				else if (colors[triangle[0]].a() >= 1.0f && colors[triangle[1]].a() >= 1.0f &&
					colors[triangle[2]].a() >= 1.0f)
				{
					framebuffer.prerenderTriangle(camera, vertices[triangle[0]],
						vertices[triangle[1]], vertices[triangle[2]]);
				}
			}
		}
		void render(Framebuffer& framebuffer, const math::PinholeCamera& camera,
			std::vector<DirectionalLight>& directionalLights,
			std::vector<PointLight>& pointLights) const
		{
			prerender(framebuffer, camera);
			if (texture)
			{
				for (const std::array<unsigned int, 3>& triangle : triangles)
				{
					framebuffer.renderTriangle(
						camera, *texture,
						vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]],
						textureCoordinates[triangle[0]],
						textureCoordinates[triangle[1]],
						textureCoordinates[triangle[2]],
						normals[triangle[0]], normals[triangle[1]], normals[triangle[2]],
						directionalLights, pointLights, material
					);
				}
			}
			else
			{
				for (const std::array<unsigned int, 3>& triangle : triangles)
				{
					framebuffer.renderTriangle(
						camera,
						vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]],
						colors[triangle[0]], colors[triangle[1]], colors[triangle[2]],
						normals[triangle[0]], normals[triangle[1]], normals[triangle[2]],
						directionalLights, pointLights, material
					);
				}
			}
		}

		void translate(const math::Vec3& direction)
		{
			for (math::Vec3& vertex : vertices)
			{
				vertex += direction;
			}
		}
		math::Vec3 getCenter() const
		{
			math::Vec3 center = math::Vec3(0.0f);
			for (const math::Vec3& vertex : vertices)
			{
				center += vertex;
			}
			return center / static_cast<float>(vertices.size());
		}
		void setCenter(const math::Vec3& center)
		{
			translate(center - getCenter());
		}

		void scale(const math::Vec3& center, const float multiplier)
		{
			for (math::Vec3& vertex : vertices)
			{
				vertex = (vertex - center) * multiplier + center;
			}
		}
		void scale(const float multiplier)
		{
			scale(getCenter(), multiplier);
		}
		float getSize(const math::Vec3& center) const
		{
			float size = 0.0f;
			for (const math::Vec3& vertex : vertices)
			{
				size += (vertex - center).norm();
			}
			return size / static_cast<float>(vertices.size());
		}
		float getSize() const
		{
			return getSize(getCenter());
		}
		void setSize(const math::Vec3& center, const float size)
		{
			scale(center, size / getSize());
		}
		void setSize(const float size)
		{
			scale(size / getSize());
		}

		void rotateAboutAxis(const math::Vec3& origin, const math::Vec3& axis, const float theta)
		{
			for (std::size_t i = 0; i < vertices.size(); i++)
			{
				const math::Vec3 out = (vertices[i] + normals[i]).rotatedAboutAxis(origin, axis,
					theta);
				vertices[i].rotateAboutAxis(origin, axis, theta);
				normals[i] = out - vertices[i];
			}
		}
		void rotateAboutAxis(const math::Vec3& axis, const float theta)
		{
			rotateAboutAxis(getCenter(), axis, theta);
		}
		void rotateAboutSegment(const math::Vec3& start, const math::Vec3& end, const float theta)
		{
			rotateAboutAxis(start, end - start, theta);
		}
	};
}
