#include "Mesh.hpp"

#include "Core/Logging.hpp"
#include "Vertex.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/gtc/type_ptr.hpp>

constexpr auto MESH_IMPORT_FLAGS = aiProcessPreset_TargetRealtime_Quality;

namespace
{
	inline glm::mat4 mat4_cast(const aiMatrix4x4& m)
	{
		return glm::transpose(glm::make_mat4(&m.a1));
	}
} // namespace

Mesh::Mesh(VkMana::Context& ctx) : m_ctx(&ctx) {}

bool Mesh::LoadFromFile(const std::filesystem::path& filename)
{
	const auto& filenameStr = filename.string();

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(filenameStr.c_str(), MESH_IMPORT_FLAGS);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG_ERR("Assimp: {}", import.GetErrorString());
		return false;
	}
	const auto rootDirectory = filename.parent_path();

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<Submesh> submeshes;
	std::vector<Material> materials;

	ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f), vertices, indices, submeshes);

	for (auto i = 0; i < scene->mNumMaterials; ++i)
	{
		const auto* material = scene->mMaterials[i];

		auto& newMaterial = materials.emplace_back();
		newMaterial.albedo = LoadMaterialTexture(material, aiTextureType_DIFFUSE, rootDirectory);
		newMaterial.normalMap = LoadMaterialTexture(material, aiTextureType_HEIGHT, rootDirectory);
		if (newMaterial.normalMap == nullptr)
			newMaterial.normalMap = LoadMaterialTexture(material, aiTextureType_NORMALS, rootDirectory);
		if (newMaterial.normalMap == nullptr)
			newMaterial.normalMap = LoadMaterialTexture(material, aiTextureType_NORMAL_CAMERA, rootDirectory);
	}

	SetVertices(vertices);
	SetIndices(indices);
	SetSubmeshes(submeshes);
	SetMaterials(materials);

	return true;
}

void Mesh::SetVertices(const std::vector<Vertex>& vertices)
{
	const auto bufferInfo = VkMana::BufferCreateInfo::Vertex(sizeof(Vertex) * vertices.size());
	const VkMana::BufferDataSource dataSrc(bufferInfo.Size, vertices.data());
	m_vertexBuffer = m_ctx->CreateBuffer(bufferInfo, &dataSrc);
}

void Mesh::SetIndices(const std::vector<uint16_t>& indices)
{
	const auto bufferInfo = VkMana::BufferCreateInfo::Index(sizeof(uint16_t) * indices.size());
	const VkMana::BufferDataSource dataSrc(bufferInfo.Size, indices.data());
	m_indexBuffer = m_ctx->CreateBuffer(bufferInfo, &dataSrc);
}

void Mesh::SetSubmeshes(const std::vector<Submesh>& submeshes)
{
	m_submeshes = submeshes;
}

void Mesh::SetMaterials(const std::vector<Material>& materials)
{
	m_materials = materials;
}

void Mesh::ProcessNode(const aiNode* node,
	const aiScene* scene,
	const glm::mat4& parentTransform,
	std::vector<Vertex>& outVertices,
	std::vector<uint16_t>& outIndices,
	std::vector<Submesh>& outSubmeshes)
{
	const auto& transform = node->mTransformation;
	auto nodeTransform = parentTransform * mat4_cast(transform);

	for (auto i = 0; i < node->mNumMeshes; ++i)
	{
		const auto* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, nodeTransform, outVertices, outIndices, outSubmeshes);
	}

	for (auto i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, nodeTransform, outVertices, outIndices, outSubmeshes);
	}
}

void Mesh::ProcessMesh(
	const aiMesh* mesh, const glm::mat4& transform, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, std::vector<Submesh>& outSubmeshes)
{
	outSubmeshes.emplace_back(Submesh{
		.indexOffset = uint32_t(outIndices.size()),
		.indexCount = mesh->mNumFaces * 3,
		.vertexOffset = uint32_t(outVertices.size()),
		.vertexCount = mesh->mNumVertices,
		.materialIndex = mesh->mMaterialIndex,
		.transform = transform,
	});

	for (auto i = 0; i < mesh->mNumVertices; ++i)
	{
		auto& vertex = outVertices.emplace_back();

		vertex.position = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z,
		};

		if (mesh->HasTextureCoords(0))
		{
			vertex.texCoord = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y,
			};
		}
		if (mesh->HasNormals())
		{
			vertex.normal = {
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z,
			};
		}
		if (mesh->HasNormals())
		{
			vertex.tangent = {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z,
			};
		}
	}

	for (auto i = 0; i < mesh->mNumFaces; ++i)
	{
		const auto& face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		for (auto j = 0; j < face.mNumIndices; ++j)
			outIndices.push_back(face.mIndices[j]);
	}
}

auto Mesh::LoadMaterialTexture(const aiMaterial* material, aiTextureType textureType, const std::filesystem::path& rootDir) const -> std::shared_ptr<Texture>
{
	if (material->GetTextureCount(textureType) == 0)
		return nullptr;

	aiString str;
	material->GetTexture(textureType, 0, &str);

	const auto textureFilename = rootDir / str.C_Str();
	auto texture = std::make_shared<Texture>(*m_ctx);
	if (!texture->LoadFromFile(textureFilename))
		return nullptr;

	return texture;
}
