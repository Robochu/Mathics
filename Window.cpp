module;
#include <glad/glad.h>
#include <GLFW/glfw3.h>

export module graphics:Window;

import :Framebuffer;

import <string>;
import <stdexcept>;

export namespace graphics
{
	constexpr int openGLMajorVersion = 2;
	constexpr int openGLMinorVersion = 1;

	// A window overtakes the entire process it's spawned in. Only a single Window instance is
	// therefore allowed to be constructed per process.
	class Window
	{
		double prev;

	protected:
		Framebuffer framebuffer;
		GLFWwindow* window;

		virtual void update(const double now, const double delta) {}
		virtual void draw() {}

		virtual void windowSizeCallback(GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
			framebuffer = graphics::Framebuffer(width, height);
		}
		virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action,
			int mods) {}
		virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

	public:
		Window(const int width, const int height, const std::string& name) :
			framebuffer(width, height)
		{
			if (glfwInit() == GLFW_FALSE)
			{
				throw std::runtime_error("Failed to initialize GLFW!");
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGLMajorVersion);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGLMinorVersion);
			window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
			if (!window)
			{
				glfwTerminate();
				throw std::runtime_error("Failed to create a GLFW window!");
			}
			glfwMakeContextCurrent(window);

			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
			{
				glfwTerminate();
				throw std::runtime_error("Failed to initialize GLAD!");
			}

			prev = glfwGetTime();
			glfwSetWindowUserPointer(window, this);
			glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
				{
					reinterpret_cast<decltype(this)>(glfwGetWindowUserPointer(window))->
						windowSizeCallback(window, width, height);
				}
			);
			glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action,
				int mods)
				{
					reinterpret_cast<decltype(this)>(glfwGetWindowUserPointer(window))->
						keyCallback(window, key, scancode, action, mods);
				}
			);
			glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action,
				int mods)
				{
					reinterpret_cast<decltype(this)>(glfwGetWindowUserPointer(window))->
						mouseButtonCallback(window, button, action, mods);
				}
			);
		}
		~Window()
		{
			glfwTerminate();
		}

		void loop()
		{
			double now;
			while (!glfwWindowShouldClose(window))
			{
				now = glfwGetTime();
				update(now, now - prev);
				prev = now;
				draw();
				framebuffer.blit();
				glfwSwapBuffers(window);
				glfwPollEvents();
			}
		}
	};
}
