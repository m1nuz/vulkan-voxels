#pragma once

#include <string>
#include <vector>

namespace Graphics {

struct TextureInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;
    std::vector<uint8_t> pixels;
    std::string name;
    std::string filepath;
};

struct TextureAtlas {
    std::vector<TextureInfo> _textures;
};

auto append_texture(TextureAtlas& atlas, std::string_view name, std::string_view filepath) -> bool;

auto get_texture_atlas(std::string_view info) -> TextureAtlas;

} // namespace Graphics