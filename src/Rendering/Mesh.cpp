#include "Mesh.hpp"

#include "Core/Logging.hpp"
#include "Vertex.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

constexpr auto MESH_IMPORT_FLAGS = aiProcessPreset_TargetRealtime_Quality;

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

	ProcessNode(scene->mRootNode, scene, vertices, indices, submeshes);

	SetVertices(vertices);
	SetIndices(indices);
	SetSubmeshes(submeshes);

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

void Mesh::ProcessNode(
	const aiNode* node, const aiScene* scene, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, std::vector<Submesh>& outSubmeshes)
{
	for (auto i = 0; i < node->mNumMeshes; ++i)
	{
		const auto* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, outVertices, outIndices, outSubmeshes);
	}

	for (auto i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, outVertices, outIndices, outSubmeshes);
	}
}

void Mesh::ProcessMesh(const aiMesh* mesh, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, std::vector<Submesh>& outSubmeshes)
{
	outSubmeshes.emplace_back(Submesh{
		.indexOffset = uint32_t(outIndices.size()),
		.indexCount = mesh->mNumFaces * 3,
		.vertexOffset = uint32_t(outVertices.size()),
		.vertexCount = mesh->mNumVertices,
		.materialIndex = mesh->mMaterialIndex,
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
