#pragma once

#include "Block.hpp"
#include "TextureAtlas.hpp"

namespace Game {

struct World;

struct Renderer {
    BlockTypes _block_types;
    Graphics::TextureAtlas _texture_atlas;
};

struct CreateRendererInfo {
    BlockTypes block_types;
    Graphics::TextureAtlas texture_atlas;
};

auto create_renderer(const CreateRendererInfo& info) -> Renderer;

auto destroy_renderer(Renderer& renderer) -> void;

auto present(Renderer& renderer, World& world) -> void;

} // namespace Game