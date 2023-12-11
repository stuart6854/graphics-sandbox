#pragma once

#include <VkMana/Context.hpp>
#include <VkMana/WSI.hpp>

class Renderer
{
public:
	bool Init(VkMana::WSI& window);

	void Flush();

private:
	VkMana::WSI* m_window = nullptr;
	VkMana::Context m_ctx{};

	VkMana::PipelineHandle m_trianglePipeline = nullptr;
};
