#include "App.hpp"

#include "Logging.hpp"

constexpr auto WINDOW_INIT_WIDTH = 1280;
constexpr auto WINDOW_INIT_HEIGHT = 720;

void App::Run()
{
	Init();

	while (m_isRunning)
	{
		m_window.NewFrame();
		if (!m_window.IsAlive())
		{
			m_isRunning = false;
			break;
		}

		m_renderer->Flush();
	}

	Cleanup();
}

void App::Init()
{
	if (!m_window.Init(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "Graphics Sandbox"))
	{
		LOG_ERR("Failed to init window");
		return;
	}

	m_renderer = std::make_unique<Renderer>();
	if (!m_renderer->Init(m_window))
	{
		LOG_ERR("Failed to init renderer");
		return;
	}

	LOG_INFO("Initialisation complete\n");

	m_isRunning = true;
}

void App::Cleanup()
{
	LOG_INFO("Cleaning up...");

	m_renderer = nullptr;
	m_window.Cleanup();
}
