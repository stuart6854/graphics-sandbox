#pragma once

#include "Mesh.hpp"

#include <VkMana/Context.hpp>
#include <VkMana/WSI.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <unordered_map>
#include <vector>

class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	bool Init(VkMana::WSI& window);

	void SetCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix);
	void Submit(Mesh* mesh, const glm::mat4& transform = glm::mat4(1.0f));

	void Flush();

	//////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////

	auto GetContext() -> auto& { return m_ctx; }

private:
	auto AddOrGetBindlessTexture(Texture* texture) -> uint32_t;
	auto AddOrGetBindlessMaterial(Material* material) -> uint32_t;
	auto AddOrGetMesh(Mesh* mesh) -> uint32_t;

	void DrawRenderInstances(VkMana::CommandBuffer& cmd);

private:
	VkMana::WSI* m_window = nullptr;
	VkMana::Context m_ctx{};

	VkMana::SetLayoutHandle m_bindlesSetLayout = nullptr;
	VkMana::SetLayoutHandle m_sceneSetLayout = nullptr;
	VkMana::SetLayoutHandle m_materialSetLayout = nullptr;

	VkMana::PipelineHandle m_trianglePipeline = nullptr;
	VkMana::PipelineHandle m_fwdMeshPipeline = nullptr; // Forward-Mesh

	//////////////////////////////////////////////////
	/// Frame Data
	//////////////////////////////////////////////////

	std::vector<const VkMana::ImageView*> m_bindlessTextures;
	std::unordered_map<Texture*, uint32_t> m_bindlessTexturesMap;

	struct SceneData
	{
		glm::mat4 projMatrix;
		glm::mat4 viewMatrix;
	} m_sceneData{};

	struct MaterialData
	{
		glm::vec4 albedoColor = { 1, 1, 1, 1 };
		uint32_t albedoTexIndex = 0;
		uint32_t normalTexIndex = 0;
	};
	std::vector<MaterialData> m_bindlessMaterials;
	std::unordered_map<Material*, uint32_t> m_bindlessMaterialsMap;

	std::vector<Mesh*> m_meshes;
	std::unordered_map<Mesh*, uint32_t> m_meshMap;

	struct RenderInstance
	{
		uint32_t meshIndex;
		uint32_t submeshIndex;
		uint32_t materialIndex;
		glm::mat4 transform;
	};
	std::vector<RenderInstance> m_renderInstances;
};
