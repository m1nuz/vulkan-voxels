#pragma once

#include "Math.hpp"

namespace Game {

struct Camera {
    mat4 _projection;
    mat4 _view;
};

} // namespace Game