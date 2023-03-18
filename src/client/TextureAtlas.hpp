#pragma once

#include <vector>

namespace Graphics {

struct TextureInfo { };

struct TextureAtlas {
    std::vector<TextureInfo> _textures;
};

} // namespace Graphics