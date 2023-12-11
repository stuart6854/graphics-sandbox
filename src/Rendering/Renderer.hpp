#pragma once

#include "Mesh.hpp"

#include <VkMana/Context.hpp>
#include <VkMana/WSI.hpp>

#include <glm/ext/matrix_float4x4.hpp>

class Renderer
{
public:
	bool Init(VkMana::WSI& window);

	void SetCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix);
	void Submit(const Mesh* mesh, const glm::mat4& transform = glm::mat4(1.0f));

	void Flush();

	//////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////

	auto GetContext() -> auto& { return m_ctx; }

private:
	void DrawRenderInstances(VkMana::CommandBuffer& cmd);

private:
	VkMana::WSI* m_window = nullptr;
	VkMana::Context m_ctx{};

	VkMana::SetLayoutHandle m_sceneSetLayout = nullptr;

	VkMana::PipelineHandle m_trianglePipeline = nullptr;
	VkMana::PipelineHandle m_fwdMeshPipeline = nullptr; // Forward-Mesh

	//////////////////////////////////////////////////
	/// Frame Data
	//////////////////////////////////////////////////

	struct SceneData
	{
		glm::mat4 projMatrix;
		glm::mat4 viewMatrix;
	} m_sceneData;

	struct RenderInstance
	{
		const Mesh* mesh = nullptr;
		glm::mat4 transform = glm::mat4(1.0f);
	};
	std::vector<RenderInstance> m_renderInstances;
};
