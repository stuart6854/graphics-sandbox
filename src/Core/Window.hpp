#pragma once

#include <VkMana/WSI.hpp>

#include <GLFW/glfw3.h>

class Window : public VkMana::WSI
{
public:
	Window() = default;
	~Window() override;

	bool Init(int32_t width, int32_t height, const char* title);
	void Cleanup();

	void NewFrame();

	void PollEvents() override;

	auto CreateSurface(vk::Instance instance) -> vk::SurfaceKHR override;

	auto GetSurfaceWidth() -> uint32_t override;
	auto GetSurfaceHeight() -> uint32_t override;

	bool IsVSync() override;
	bool IsAlive() override;

	void HideCursor() override;
	void ShowCursor() override;

	auto CreateCursor(uint32_t cursorType) -> void* override;
	void SetCursor(void* cursor) override;

private:
	GLFWwindow* m_window;
};
