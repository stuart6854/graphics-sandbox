#pragma once

#include "Texture.hpp"

#include <memory>

struct Material
{
	std::shared_ptr<Texture> albedo = nullptr;
	std::shared_ptr<Texture> normalMap = nullptr;
};