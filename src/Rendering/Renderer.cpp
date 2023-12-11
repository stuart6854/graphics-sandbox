#include "Renderer.hpp"

#include "Core/Logging.hpp"

#include <VKMana/ShaderCompiler.hpp>

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
			.Layout = pipelineLayout.Get(),
		};
		m_trianglePipeline = m_ctx.CreateGraphicsPipeline(pipelineInfo);
	}

	return true;
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
	mainCmd->EndRenderPass();

	m_ctx.Submit(mainCmd);

	m_ctx.EndFrame();
	m_ctx.Present();
}
