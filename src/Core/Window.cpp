#include "Window.hpp"

Window::~Window()
{
	glfwTerminate();
}

bool Window::Init(int32_t width, int32_t height, const char* title)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!m_window)
		return false;

	glfwSetWindowUserPointer(m_window, this);

	return true;
}

void Window::Cleanup()
{
	glfwDestroyWindow(m_window);
	m_window = nullptr;
}

void Window::NewFrame()
{
	PollEvents();
}

void Window::PollEvents()
{
	glfwPollEvents();
}

auto Window::CreateSurface(const vk::Instance instance) -> vk::SurfaceKHR
{
	VkSurfaceKHR surface = nullptr;
	glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
	return surface;
}

auto Window::GetSurfaceWidth() -> uint32_t
{
	int32_t width;
	int32_t height;
	glfwGetFramebufferSize(m_window, &width, &height);
	return width;
}

auto Window::GetSurfaceHeight() -> uint32_t
{
	int32_t width;
	int32_t height;
	glfwGetFramebufferSize(m_window, &width, &height);
	return height;
}

bool Window::IsVSync()
{
	return true;
}

bool Window::IsAlive()
{
	return !glfwWindowShouldClose(m_window);
}

void Window::HideCursor() {}

void Window::ShowCursor() {}

auto Window::CreateCursor(uint32_t /*cursorType*/) -> void*
{
	return nullptr;
}

void Window::SetCursor(void* cursor) {}
