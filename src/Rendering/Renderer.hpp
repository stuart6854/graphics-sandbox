#pragma once

#include "Mesh.hpp"

#include <VkMana/CommandBuffer.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/Descriptors.hpp>
#include <VkMana/Image.hpp>
#include <VkMana/Pipeline.hpp>
#include <VkMana/RenderPass.hpp>
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

	bool SetupGBufferPass();

	void ExecGBufferPass(VkMana::CommandBuffer& cmd, VkMana::DescriptorSet& bindlessSet);

	void DrawRenderInstances(VkMana::CommandBuffer& cmd);

private:
	VkMana::WSI* m_window = nullptr;
	VkMana::Context m_ctx{};

	std::unique_ptr<Texture> m_whiteTexture = nullptr;
	std::unique_ptr<Texture> m_blackTexture = nullptr;

	VkMana::ImageHandle m_depthTarget = nullptr;

#pragma region G-Buffer
	VkMana::ImageHandle m_positionGBufTarget = nullptr;
	VkMana::ImageHandle m_normalGBufTarget = nullptr;
	VkMana::ImageHandle m_albedoGBufTarget = nullptr;
	VkMana::RenderPassInfo m_gBufRenderPass;

	VkMana::PipelineHandle m_gBufPipeline = nullptr;

#pragma endregion

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

#pragma pack(push, 4)
	struct MaterialData
	{
		glm::vec4 albedoColor = { 1, 1, 1, 1 };
		uint32_t albedoTexIndex = 0;
		uint32_t normalTexIndex = 0;
		float padding[2];
	};
#pragma pack(pop)
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
