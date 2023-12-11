#pragma once

#include "Rendering/Renderer.hpp"
#include "Window.hpp"

#include <memory>

class App
{
public:
	App() = default;
	~App() = default;

	void Run();

private:
	void Init();
	void Cleanup();

private:
	bool m_isRunning = false;
	Window m_window;
	std::unique_ptr<Renderer> m_renderer;
};
