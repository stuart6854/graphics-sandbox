#pragma once

#include "Material.hpp"
#include "Submesh.hpp"
#include "Vertex.hpp"

#include <filesystem>
#include <vector>

#include <VkMana/Buffer.hpp>
#include <VkMana/Context.hpp>

#include <assimp/scene.h>

class Mesh
{
public:
	explicit Mesh(VkMana::Context& ctx);
	~Mesh() = default;

	bool LoadFromFile(const std::filesystem::path& filename);

	void SetVertices(const std::vector<Vertex>& vertices);
	void SetIndices(const std::vector<uint16_t>& indices);
	void SetSubmeshes(const std::vector<Submesh>& submeshes);
	void SetMaterials(const std::vector<Material>& materials);

	//////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////

	auto GetVertexBuffer() const -> const auto& { return m_vertexBuffer; }
	auto GetIndexBuffer() const -> const auto& { return m_indexBuffer; }
	auto GetSubmeshes() const -> const auto& { return m_submeshes; }
	auto GetMaterials() -> auto& { return m_materials; }

private:
	static void ProcessNode(
		const aiNode* node, const aiScene* scene, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, std::vector<Submesh>& outSubmeshes);
	static void ProcessMesh(const aiMesh* mesh, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, std::vector<Submesh>& outSubmeshes);

	auto LoadMaterialTexture(const aiMaterial* material, aiTextureType textureType, const std::filesystem::path& rootDir) const -> std::shared_ptr<Texture>;

private:
	VkMana::Context* m_ctx = nullptr;

	VkMana::BufferHandle m_vertexBuffer = nullptr;
	VkMana::BufferHandle m_indexBuffer = nullptr;
	std::vector<Submesh> m_submeshes;
	std::vector<Material> m_materials;
};
