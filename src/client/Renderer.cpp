#include "Renderer.hpp"

namespace Game {

auto create_renderer([[maybe_unused]] const CreateRendererInfo& info) -> Renderer {
    Renderer renderer;
    renderer._block_types = info.block_types;
    renderer._texture_atlas = info.texture_atlas;

    return renderer;
}

auto destroy_renderer([[maybe_unused]] Renderer& renderer) -> void {
}

auto present([[maybe_unused]] Renderer& renderer, [[maybe_unused]] World& world) -> void {
}

} // namespace Game