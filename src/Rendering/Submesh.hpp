#pragma once

#include <cstdint>

#include <glm/ext/matrix_float4x4.hpp>

struct Submesh
{
	uint32_t indexOffset = 0;
	uint32_t indexCount = 0;
	uint32_t vertexOffset = 0;
	uint32_t vertexCount = 0;
	uint32_t materialIndex = 0;
	glm::mat4 transform = glm::mat4(1.0f);
};
