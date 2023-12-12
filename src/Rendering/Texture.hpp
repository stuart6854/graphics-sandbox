#pragma once

#include <VkMana/Image.hpp>

#include <filesystem>

class Texture
{
public:
	explicit Texture(VkMana::Context& ctx);
	~Texture() = default;

	bool LoadFromFile(const std::filesystem::path& filename);

	auto GetImage() -> auto& { return m_image; }
	auto GetImage() const -> const auto& { return m_image; }

private:
	VkMana::Context* m_ctx = nullptr;
	VkMana::ImageHandle m_image = nullptr;
};
