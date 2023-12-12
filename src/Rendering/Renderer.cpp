#include "Renderer.hpp"

#include "Core/Logging.hpp"

#include <VKMana/ShaderCompiler.hpp>

#include <glm/gtc/type_ptr.hpp>

const auto TriangleHLSLShader = R"(
struct VSOutput
{
	float4 FragPos : SV_POSITION;
	[[vk::location(0)]] float4 Color : COLOR0;
};

VSOutput VSMain(uint vtxId : SV_VERTEXID)
{
	const float3 positions[3] = {
		float3(0.5, 0.5, 0.0),
		float3(-0.5, 0.5, 0.0),
		float3(0.0, -0.5, 0.0),
	};

	const float4 colors[3] = {
		float4(1, 0, 0, 1),
		float4(0, 1, 0, 1),
		float4(0, 0, 1, 1),
	};

	VSOutput output;
	output.FragPos = float4(positions[vtxId], 1.0);
	output.Color = colors[vtxId];
	return output;
}

struct PSInput
{
	[[vk::location(0)]] float4 Color : COLOR;
};

struct PSOutput
{
	float4 FragColor : SV_TARGET0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput output;
	output.FragColor = input.Color;
	return output;
}
)";

bool Renderer::Init(VkMana::WSI& window)
{
	m_window = &window;

	if (!m_ctx.Init(m_window))
	{
		LOG_ERR("Failed to init VkMana context");
		return false;
	}
	{
		// Bindless set layout
		std::vector bindings{
			VkMana::SetLayoutBinding(
				0, vk::DescriptorType::eCombinedImageSampler, 10000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound),
		};
		m_bindlesSetLayout = m_ctx.CreateSetLayout(bindings);
	}
	{
		// Scene set layout
		std::vector bindings{
			VkMana::SetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
		};
		m_sceneSetLayout = m_ctx.CreateSetLayout(bindings);
	}
	{
		// Material set layout
		std::vector bindings{
			VkMana::SetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment),
		};
		m_materialSetLayout = m_ctx.CreateSetLayout(bindings);
	}
	{
		// Triangle Pipeline
		const VkMana::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		auto pipelineLayout = m_ctx.CreatePipelineLayout(pipelineLayoutInfo);

		VkMana::ShaderCompileInfo compileInfo{
			.SrcLanguage = VkMana::SourceLanguage::HLSL,
			.SrcString = TriangleHLSLShader,
			.Stage = vk::ShaderStageFlagBits::eVertex,
			.EntryPoint = "VSMain",
			.Debug = false,
		};
		const auto vertSpirvOpt = VkMana::CompileShader(compileInfo);
		if (!vertSpirvOpt)
		{
			VM_ERR("Failed to compiler VERTEX shader.");
			return false;
		}

		compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
		compileInfo.EntryPoint = "PSMain";
		const auto fragSpirvOpt = VkMana::CompileShader(compileInfo);
		if (!fragSpirvOpt)
		{
			VM_ERR("Failed to compiler FRAGMENT shader.");
			return false;
		}

		const VkMana::GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = { vertSpirvOpt.value(), "VSMain" },
			.Fragment = { fragSpirvOpt.value(), "PSMain" },
			.Topology = vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
			.Layout = pipelineLayout,
		};
		m_trianglePipeline = m_ctx.CreateGraphicsPipeline(pipelineInfo);
	}
	{
		// Foward-Mesh Pipeline

		const VkMana::PipelineLayoutCreateInfo pipelineLayoutInfo{
			.PushConstantRange = { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0u, uint32_t(sizeof(glm::mat4) + sizeof(uint32_t)) },
			.SetLayouts = { m_bindlesSetLayout.Get(), m_sceneSetLayout.Get(), m_materialSetLayout.Get(), },
		};
		auto pipelineLayout = m_ctx.CreatePipelineLayout(pipelineLayoutInfo);

		VkMana::ShaderCompileInfo compileInfo{
			.SrcLanguage = VkMana::SourceLanguage::HLSL,
			.SrcFilename = "assets/shaders/fwd_mesh.hlsl",
			.Stage = vk::ShaderStageFlagBits::eVertex,
			.EntryPoint = "VSMain",
			.Debug = false,
		};
		const auto vertSpirvOpt = VkMana::CompileShader(compileInfo);
		if (!vertSpirvOpt)
		{
			VM_ERR("Failed to compiler VERTEX shader.");
			return false;
		}

		compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
		compileInfo.EntryPoint = "PSMain";
		const auto fragSpirvOpt = VkMana::CompileShader(compileInfo);
		if (!fragSpirvOpt)
		{
			VM_ERR("Failed to compiler FRAGMENT shader.");
			return false;
		}

		const VkMana::GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = { vertSpirvOpt.value(), "VSMain" },
			.Fragment = { fragSpirvOpt.value(), "PSMain" },
			.VertexAttributes = {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)),
			},
			.VertexBindings = {
				vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex),
			},
			.Topology = vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
			.Layout = pipelineLayout,
		};
		m_fwdMeshPipeline = m_ctx.CreateGraphicsPipeline(pipelineInfo);
	}

	return true;
}

void Renderer::SetCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix)
{
	m_sceneData.projMatrix = projMatrix;
	m_sceneData.viewMatrix = viewMatrix;
}

void Renderer::Submit(Mesh* mesh, const glm::mat4& transform)
{
	const auto& submeshes = mesh->GetSubmeshes();
	auto& materials = mesh->GetMaterials();
	for (auto i = 0; i < submeshes.size(); ++i)
	{
		const auto& submesh = submeshes[i];
		auto& material = materials[submesh.materialIndex];

		auto& renderInstance = m_renderInstances.emplace_back();
		renderInstance.meshIndex = AddOrGetMesh(mesh);
		renderInstance.submeshIndex = i;
		renderInstance.materialIndex = AddOrGetBindlessMaterial(&material);
		renderInstance.transform = transform * submesh.transform;
	}
}

void Renderer::Flush()
{
	const auto windowWidth = m_window->GetSurfaceWidth();
	const auto windowHeight = m_window->GetSurfaceHeight();

	m_ctx.BeginFrame();

	auto bindlessSet = m_ctx.RequestDescriptorSet(m_bindlesSetLayout.Get());
	m_ctx.SetName(*bindlessSet, "descriptor_set_bindless");
	bindlessSet->WriteArray(0, 0, m_bindlessTextures, m_ctx.GetLinearSampler());

	auto mainCmd = m_ctx.RequestCmd();

	const auto rpInfo = m_ctx.GetSurfaceRenderPass(m_window);
	mainCmd->BeginRenderPass(rpInfo);
	// mainCmd->BindPipeline(m_trianglePipeline.Get());
	// mainCmd->SetViewport(0, 0, float(windowWidth), float(windowHeight));
	// mainCmd->SetScissor(0, 0, windowWidth, windowHeight);
	// mainCmd->Draw(3, 0);

	{

		/* Scene Set */
		auto sceneUbo = m_ctx.CreateBuffer(VkMana::BufferCreateInfo::Uniform(sizeof(SceneData)));
		m_ctx.SetName(*sceneUbo, "ubo_scene");
		sceneUbo->WriteHostAccessible(0, sizeof(SceneData), &m_sceneData);

		auto sceneSet = m_ctx.RequestDescriptorSet(m_sceneSetLayout.Get());
		m_ctx.SetName(*sceneSet, "descriptor_set_scene");
		sceneSet->Write(sceneUbo.Get(), 0, vk::DescriptorType::eUniformBuffer, 0, sceneUbo->GetSize());

		/* Materials Set */
		auto materialUbo = m_ctx.CreateBuffer(VkMana::BufferCreateInfo::Uniform(sizeof(MaterialData) * m_bindlessMaterials.size()));
		m_ctx.SetName(*materialUbo, "ubo_materials");
		materialUbo->WriteHostAccessible(0, sizeof(MaterialData) * m_bindlessMaterials.size(), m_bindlessMaterials.data());

		auto materialSet = m_ctx.RequestDescriptorSet(m_materialSetLayout.Get());
		m_ctx.SetName(*materialSet, "descriptor_set_materials");
		materialSet->Write(materialUbo.Get(), 0, vk::DescriptorType::eUniformBuffer, 0, materialUbo->GetSize());

		mainCmd->BindPipeline(m_fwdMeshPipeline.Get());
		mainCmd->SetViewport(0.0f, float(windowHeight), float(windowWidth), -float(windowHeight), 0.0f, 1.0f);
		mainCmd->SetScissor(0, 0, windowWidth, windowHeight);

		mainCmd->BindDescriptorSets(0, { bindlessSet.Get(), sceneSet.Get(), materialSet.Get() }, {});

		DrawRenderInstances(*mainCmd);
	}

	mainCmd->EndRenderPass();

	m_ctx.Submit(mainCmd);

	m_ctx.EndFrame();
	m_ctx.Present();

	m_renderInstances.clear();
}

auto Renderer::AddOrGetBindlessTexture(Texture* texture) -> uint32_t
{
	const auto it = m_bindlessTexturesMap.find(texture);
	if (it != m_bindlessTexturesMap.end())
		return it->second;

	const auto index = m_bindlessTextures.size();
	m_bindlessTextures.push_back(texture->GetImage()->GetImageView(VkMana::ImageViewType::Texture));
	m_bindlessTexturesMap[texture] = index;
	return index;
}

auto Renderer::AddOrGetBindlessMaterial(Material* material) -> uint32_t
{
	const auto it = m_bindlessMaterialsMap.find(material);
	if (it != m_bindlessMaterialsMap.end())
		return it->second;

	const auto index = m_bindlessMaterials.size();
	m_bindlessMaterialsMap[material] = index;

	auto& materialData = m_bindlessMaterials.emplace_back();
	materialData.albedoTexIndex = AddOrGetBindlessTexture(material->albedo.get());
	materialData.normalTexIndex = AddOrGetBindlessTexture(material->normalMap.get());

	return index;
}

auto Renderer::AddOrGetMesh(Mesh* mesh) -> uint32_t
{
	const auto it = m_meshMap.find(mesh);
	if (it != m_meshMap.end())
		return it->second;

	const auto index = m_meshes.size();
	m_meshes.push_back(mesh);
	m_meshMap[mesh] = index;
	return index;
}

void Renderer::DrawRenderInstances(VkMana::CommandBuffer& cmd)
{
	for (const auto& instance : m_renderInstances)
	{
		const auto* mesh = m_meshes[instance.meshIndex];
		// #TODO: Cache bound mesh
		cmd.BindVertexBuffers(0, { mesh->GetVertexBuffer().Get() }, { 0 });
		cmd.BindIndexBuffer(mesh->GetIndexBuffer().Get());

		cmd.SetPushConstants(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(glm::mat4), glm::value_ptr(instance.transform));
		cmd.SetPushConstants(
			vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4), sizeof(uint32_t), &instance.materialIndex);

		const auto& submesh = mesh->GetSubmeshes().at(instance.submeshIndex);
		cmd.DrawIndexed(submesh.indexCount, submesh.indexOffset, submesh.vertexOffset);
	}
}
