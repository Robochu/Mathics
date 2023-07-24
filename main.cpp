import math;
import color;
import graphics;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <numbers>
#include <cstddef>

class Mathics : public graphics::Window
{
	static constexpr double frequency = 0.5f;

	math::PinholeCamera camera;
	std::vector<graphics::TriangleMesh> meshes;
	std::vector<graphics::DirectionalLight> directionalLights;
	std::vector<graphics::PointLight> pointLights;
	std::vector<graphics::Framebuffer> textures;
	graphics::CubeMap skyBox;
	graphics::CubeMap reflection;

	unsigned int fps;
	double prev;

public:
	void update(const double now, const double delta) override
	{
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			double x;
			double y;
			glfwGetCursorPos(window, &x, &y);
			camera = math::PinholeCamera(
				static_cast<unsigned int>(framebuffer.getWidth()),
				static_cast<unsigned int>(framebuffer.getHeight()), math::toRadians(70.0f),
				camera.center, {
					static_cast<float>(std::cos(x * 0.005)) *
						static_cast<float>(std::sin(y * 0.005)),
					static_cast<float>(std::cos(y * 0.005)),
					static_cast<float>(std::sin(x * 0.005)) *
						static_cast<float>(std::sin(y * 0.005))
				}, {
					static_cast<float>(std::cos(x * 0.005)) *
						static_cast<float>(-std::cos(y * 0.005)),
					static_cast<float>(std::sin(y * 0.005)),
					static_cast<float>(std::sin(x * 0.005)) *
						static_cast<float>(-std::cos(y * 0.005))
				}
			);
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			math::Vec3 direction = camera.getViewDirection();
			direction.y() = 0.0f;
			direction.normalize();
			camera.center += 100.0f * static_cast<float>(delta) * direction;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			math::Vec3 direction = camera.getViewDirection();
			direction.y() = 0.0f;
			direction.normalize();
			camera.center -= 100.0f * static_cast<float>(delta) * direction;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.center += 100.0f * static_cast<float>(delta) * camera.a;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.center -= 100.0f * static_cast<float>(delta) * camera.a;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			camera.center += 100.0f * static_cast<float>(delta) * math::Vec3(0.0f, 1.0f, 0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			camera.center -= 100.0f * static_cast<float>(delta) * math::Vec3(0.0f, 1.0f, 0.0f);
		}

		pointLights[0].setPosition({
			75.0f * static_cast<float>(std::sin(now)),
			50.0f,
			200.0f + 75.0f * static_cast<float>(std::cos(now))
		});
		pointLights[1].setPosition({
			75.0f * static_cast<float>(std::sin(now)),
			50.0f,
			-200.0f + 75.0f * static_cast<float>(std::cos(now))
		});

		if (now - prev > frequency)
		{
			fps = static_cast<unsigned int>(fps / frequency);
			std::cout << "\x1B[2J\x1B[H";
			if (fps <= 6)
			{
				std::cout << "\x1B[31m";
			}
			else if (fps <= 12)
			{
				std::cout << "\x1B[91m";
			}
			else if (fps <= 24)
			{
				std::cout << "\x1B[33m";
			}
			else if (fps <= 60)
			{
				std::cout << "\x1B[93m";
			}
			else if (fps <= 120)
			{
				std::cout << "\x1B[32m";
			}
			else
			{
				std::cout << "\x1B[92m";
			}
			std::cout << "FPS: " << fps << "\033[0m";
			fps = 0;
			prev = now;
		}
		fps++;
	}
	void draw() override
	{
		skyBox.renderOnto(framebuffer, camera);
		framebuffer.zClear();
		for (graphics::PointLight& pointLight : pointLights)
		{
			pointLight.shadowMap.zClear();
			for (const graphics::TriangleMesh& mesh : meshes)
			{
				pointLight.shadowMap.prerender(mesh);
			}
		}

		meshes[0].render(framebuffer, camera, directionalLights, pointLights);
		skyBox.renderOnto(reflection);
		reflection.setPosition({0.0f, 25.0f, 200.0f});
		for (std::size_t i = 0; i < 5; i++)
		{
			if (i != 1)
			{
				reflection.render(meshes[i], directionalLights, pointLights);
			}
		}
		graphics::reflectionMap = &reflection;
		meshes[1].render(framebuffer, camera, directionalLights, pointLights);

		skyBox.renderOnto(reflection);
		reflection.setPosition({0.0f, 25.0f, 0.0f});
		for (std::size_t i = 0; i < 5; i++)
		{
			if (i != 2)
			{
				reflection.render(meshes[i], directionalLights, pointLights);
			}
		}
		graphics::reflectionMap = &reflection;
		meshes[2].render(framebuffer, camera, directionalLights, pointLights);
		meshes[3].render(framebuffer, camera, directionalLights, pointLights);
		meshes[4].render(framebuffer, camera, directionalLights, pointLights);
	}

	void windowSizeCallback(GLFWwindow* window, int width, int height) override
	{
		Window::windowSizeCallback(window, width, height);
		camera.resize(width, height);
	}
	void keyCallback(GLFWwindow* window, int key, int scancode, int action,
		int mods) override
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			}
		}
	}
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override
	{
		if (action == GLFW_PRESS)
		{
			switch (button)
			{
			case GLFW_MOUSE_BUTTON_1:
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				if (glfwRawMouseMotionSupported())
				{
					glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
				}
			}
			}
		}
	}

	Mathics(unsigned int width, unsigned int height) : Window(width, height, "Mathics"),
		camera(width, height, math::toRadians(70.0f), {200.0f, 50.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f}), meshes(5), reflection(128, math::Vec3(0.0f)), fps(0),
		prev(glfwGetTime())
	{
		textures.push_back(graphics::Framebuffer("grass.tiff"));
		textures.push_back(graphics::Framebuffer("metal.tiff"));

		meshes[0].texture = &textures[0];
		meshes[0].addQuad(
			{-100.0f, 0.0f, 100.0f},
			{-100.0f, 0.0f, 300.0f},
			{100.0f, 0.0f, 300.0f},
			{100.0f, 0.0f, 100.0f},
			{0.0f, 0.0f}, {0.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 0.0f}
		);

		meshes[1] = graphics::TriangleMesh("teapot1K.bin", &textures[1], graphics::reflective);
		meshes[1].setCenter({0.0f, 25.0f, 200.0f});

		meshes[2] = graphics::TriangleMesh("teapot1K.bin", graphics::specularChrome);
		meshes[2].setCenter({0.0f, 25.0f, 0.0f});

		meshes[3].texture = &textures[0];
		meshes[3].addQuad(
			{-100.0f, 0.0f, -300.0f},
			{-100.0f, 0.0f, -100.0f},
			{100.0f, 0.0f, -100.0f},
			{100.0f, 0.0f, -300.0f},
			{0.0f, 0.0f}, {0.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 0.0f}
		);

		meshes[4] = graphics::TriangleMesh("teapot1K.bin", graphics::shiny);
		meshes[4].setCenter({0.0f, 25.0f, -200.0f});
		for (math::Vec4& color : meshes[4].colors)
		{
			color.a() = 0.5f;
		}

		directionalLights.push_back(graphics::DirectionalLight({0.0f, 1.0f, 0.0f}, 0.1f));
		pointLights.push_back(graphics::PointLight(512, {75.0f, 50.0f, 250.0f}, 10000.0f,
			100.0f, color::lemonYellowCrayola.subvector<3>()));
		pointLights.push_back(graphics::PointLight(512, {75.0f, 50.0f, -250.0f}, 10000.0f,
			100.0f, color::lemonYellowCrayola.subvector<3>()));

		skyBox = graphics::CubeMap({
			graphics::Framebuffer("bk.tiff"),
			graphics::Framebuffer("up.tiff"),
			graphics::Framebuffer("lf.tiff"),
			graphics::Framebuffer("ft.tiff"),
			graphics::Framebuffer("dn.tiff"),
			graphics::Framebuffer("rt.tiff")
		});
	}
};

int main(int argc, char* argv[])
{
	Mathics window = Mathics(1000, 600);
	window.loop();
	return 0;
}
