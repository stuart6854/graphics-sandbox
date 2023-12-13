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

#include <array>
#include <unordered_map>
#include <vector>

class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	bool Init(VkMana::WSI& window);

	void SetCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix);
	void SubmitAmbientLight(const glm::vec3& color = { 1, 1, 1 }, float intensity = 0.1f);
	void SubmitDirectionalLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float intensity = 1.0);
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
	bool SetupCompositePass();

	void ExecGBufferPass(VkMana::CommandBuffer& cmd, VkMana::DescriptorSet& bindlessSet);
	void ExecCompositePass(VkMana::CommandBuffer& cmd);

	void DrawRenderInstances(VkMana::CommandBuffer& cmd);

private:
	VkMana::WSI* m_window = nullptr;
	VkMana::Context m_ctx{};

	std::unique_ptr<Texture> m_whiteTexture = nullptr;
	std::unique_ptr<Texture> m_blackTexture = nullptr;

	VkMana::ImageHandle m_depthTarget = nullptr;

	VkMana::SetLayoutHandle m_bindlesSetLayout = nullptr;
	VkMana::SetLayoutHandle m_cameraSetLayout = nullptr;
	VkMana::SetLayoutHandle m_materialSetLayout = nullptr;

#pragma region G-Buffer
	VkMana::ImageHandle m_positionGBufTarget = nullptr;
	VkMana::ImageHandle m_normalGBufTarget = nullptr;
	VkMana::ImageHandle m_albedoGBufTarget = nullptr;
	VkMana::RenderPassInfo m_gBufRenderPass;

	VkMana::PipelineHandle m_gBufPipeline = nullptr;
#pragma endregion

#pragma region Composite
	VkMana::SetLayoutHandle m_gBufTargetSetLayout = nullptr;
	VkMana::SetLayoutHandle m_lightingSetLayout = nullptr;
	VkMana::PipelineHandle m_compositePipeline = nullptr;
#pragma endregion

	//////////////////////////////////////////////////
	/// Frame Data
	//////////////////////////////////////////////////

	std::vector<const VkMana::ImageView*> m_bindlessTextures;
	std::unordered_map<Texture*, uint32_t> m_bindlessTexturesMap;

	struct CameraData
	{
		glm::mat4 projMatrix;
		glm::mat4 viewMatrix;
	} m_cameraData{};

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

#pragma pack(push, 4)
	struct Light
	{
		glm::vec4 position{};
		glm::vec4 direction{};
		glm::vec4 color{ 1, 1, 1, 0 }; // XYZ=Color, W=Intensity
	};
	struct LightingData
	{
		glm::vec4 ambientLight = { 1, 1, 1, 0.1 }; // XYZ=Color, W=Strength
		std::array<Light, 16> lights{};
	} m_lightingData{};
#pragma pack(pop)
	uint32_t m_lightCount = 0;
};
