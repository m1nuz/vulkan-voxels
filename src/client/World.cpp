#include "World.hpp"

namespace Game {

auto create_world() -> World {
    World world;

    return world;
}

auto update_world([[maybe_unused]] World& world) -> void {
}

} // namespace Game