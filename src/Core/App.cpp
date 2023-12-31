#include "App.hpp"

#include "Logging.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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

		/* Render */
		const auto windowAspect = float(m_window.GetSurfaceWidth()) / float(m_window.GetSurfaceHeight());
		const auto projMatrix = glm::perspectiveLH_ZO(glm::radians(60.0f), windowAspect, 0.1f, 1000.0f);
		const auto viewMatrix = glm::lookAtLH(glm::vec3(-0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0, 1, 0));
		m_renderer->SetCamera(projMatrix, viewMatrix);

		auto backpackTransform = glm::translate(glm::mat4(1.0f), { -3.0f, 0, -2.0f }) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f))
			* glm::rotate(glm::mat4(1.0f), glm::radians(210.0f), { 0, 1, 0 });
		m_renderer->Submit(m_backpackMesh.get(), backpackTransform);

		auto runestoneTransform = glm::translate(glm::mat4(1.0f), { 0, -5, 5 }) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
		m_renderer->Submit(m_runestoneMesh.get(), runestoneTransform);

		m_renderer->Flush();
	}
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

	m_backpackMesh = std::make_unique<Mesh>(m_renderer->GetContext());
	if (!m_backpackMesh->LoadFromFile("assets/models/backpack/scene.gltf"))
	{
		LOG_ERR("Failed to load backpack model.");
	}
	m_runestoneMesh = std::make_unique<Mesh>(m_renderer->GetContext());
	if (!m_runestoneMesh->LoadFromFile("assets/models/runestone/scene.gltf"))
	{
		LOG_ERR("Failed to load backpack model.");
	}

	LOG_INFO("Initialisation complete\n");

	m_isRunning = true;
}
