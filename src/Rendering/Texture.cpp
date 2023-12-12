#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <VkMana/Context.hpp>
#include <stb_image.h>

Texture::Texture(VkMana::Context& ctx) : m_ctx(&ctx) {}

bool Texture::LoadFromFile(const std::filesystem::path& filename)
{
	const auto& filenameStr = filename.string();

	stbi_set_flip_vertically_on_load(true);

	int32_t w = 0;
	int32_t h = 0;
	int32_t c = 0;
	const auto* pixels = stbi_load(filenameStr.c_str(), &w, &h, &c, 4);
	c = 4;

	if (pixels == nullptr)
	{
		return false;
	}

	const auto imageInfo = VkMana::ImageCreateInfo::Texture(w, h);
	const VkMana::ImageDataSource dataSrc{ uint32_t(w * h * c), pixels };
	m_image = m_ctx->CreateImage(imageInfo, &dataSrc);

	return m_image != nullptr;
}

bool Texture::FromData(uint32_t width, uint32_t height, const void* data)
{
	if (width == 0 && height == 0)
		return false;
	if (data == nullptr)
		return false;

	const auto imageInfo = VkMana::ImageCreateInfo::Texture(width, height);
	const VkMana::ImageDataSource dataSrc{ uint32_t(width * height * 4), data };
	m_image = m_ctx->CreateImage(imageInfo, &dataSrc);

	return m_image != nullptr;
}
