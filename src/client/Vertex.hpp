#pragma once

#include "Math.hpp"

namespace Game {

struct Vertex {
    vec3 position = vec3 { 0, 0, 0 };
    vec3 color = vec3 { 0, 0, 0 };
    vec3 texcoord = vec3 { 0, 0, 0 };
};

} // namespace Game