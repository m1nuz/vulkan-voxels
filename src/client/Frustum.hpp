#pragma once

#include "Plane.hpp"

namespace Game {

struct Frustum {
    Plane nearPlan;
    Plane farPlane;
    Plane left;
    Plane right;
    Plane top;
    Plane bottom;
};

} // namespace Game