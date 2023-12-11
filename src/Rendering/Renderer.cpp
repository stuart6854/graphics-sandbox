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
		// Scene set layout
		std::vector bindings{
			VkMana::SetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
		};
		m_sceneSetLayout = m_ctx.CreateSetLayout(bindings);
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
			.PushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0u, uint32_t(sizeof(glm::mat4)) },
			.SetLayouts = { m_sceneSetLayout.Get() },
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
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, 0),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, 0),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, 0),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, 0),
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

void Renderer::Submit(const Mesh* mesh, const glm::mat4& transform)
{
	m_renderInstances.emplace_back(mesh, transform);
}

void Renderer::Flush()
{
	const auto windowWidth = m_window->GetSurfaceWidth();
	const auto windowHeight = m_window->GetSurfaceHeight();

	m_ctx.BeginFrame();

	auto mainCmd = m_ctx.RequestCmd();

	const auto rpInfo = m_ctx.GetSurfaceRenderPass(m_window);
	mainCmd->BeginRenderPass(rpInfo);
	mainCmd->BindPipeline(m_trianglePipeline.Get());
	mainCmd->SetViewport(0, 0, float(windowWidth), float(windowHeight));
	mainCmd->SetScissor(0, 0, windowWidth, windowHeight);
	mainCmd->Draw(3, 0);

	{
		auto sceneUbo = m_ctx.CreateBuffer(VkMana::BufferCreateInfo::Uniform(sizeof(SceneData)));
		m_ctx.SetName(*sceneUbo, "ubo_scene");
		sceneUbo->WriteHostAccessible(0, sizeof(SceneData), &m_sceneData);

		auto sceneSet = m_ctx.RequestDescriptorSet(m_sceneSetLayout.Get());
		m_ctx.SetName(*sceneSet, "descriptor_set_scene");
		sceneSet->Write(sceneUbo.Get(), 0, vk::DescriptorType::eUniformBuffer, 0, sizeof(SceneData));

		mainCmd->BindPipeline(m_fwdMeshPipeline.Get());
		mainCmd->SetViewport(0.0f, float(windowHeight), float(windowWidth), -float(windowHeight), 0.0f, 1.0f);
		mainCmd->SetScissor(0, 0, windowWidth, windowHeight);

		const std::vector sets = { sceneSet.Get() };
		mainCmd->BindDescriptorSets(0, sets, {});

		DrawRenderInstances(*mainCmd);
	}

	mainCmd->EndRenderPass();

	m_ctx.Submit(mainCmd);

	m_ctx.EndFrame();
	m_ctx.Present();

	m_renderInstances.clear();
}

void Renderer::DrawRenderInstances(VkMana::CommandBuffer& cmd)
{
	for (const auto& instance : m_renderInstances)
	{
		const auto* mesh = instance.mesh;
		cmd.BindVertexBuffers(0, { mesh->GetVertexBuffer().Get() }, { 0 });
		cmd.BindIndexBuffer(mesh->GetIndexBuffer().Get());

		const auto& submeshes = mesh->GetSubmeshes();
		for (const auto& submesh : submeshes)
		{
			const auto modelMatrix = submesh.transform * instance.transform;

			cmd.SetPushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), glm::value_ptr(modelMatrix));
			cmd.DrawIndexed(submesh.indexCount, submesh.indexOffset, submesh.vertexOffset);
		}
	}
}
