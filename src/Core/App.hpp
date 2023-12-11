#pragma once

#include "Rendering/Mesh.hpp"
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

private:
	bool m_isRunning = false;
	Window m_window;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Mesh> m_backpackMesh;
};
