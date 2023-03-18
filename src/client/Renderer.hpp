#pragma once

namespace Game {

struct World;

struct Renderer { };

auto present(Renderer& renderer, World& world) -> void;

} // namespace Game