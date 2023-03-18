#pragma once

#include <vector>

#include "Camera.hpp"
#include "Chunk.hpp"

namespace Game {

struct World {
    std::vector<Chunk> _chunks;
    Camera _camera;
};

auto create_world() -> World;
auto update_world(World& world) -> void;

} // namespace Game