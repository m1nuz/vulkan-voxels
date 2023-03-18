#pragma once

#include "Math.hpp"

#include <vector>

namespace Game {

struct BlockType {
    uint32_t frontTexture = 0;
    uint32_t leftTexture = 0;
    uint32_t rightTexture = 0;
    uint32_t backTexture = 0;
    uint32_t topTexture = 0;
    uint32_t bottomTexture = 0;

    vec3 frontColor = { 1.0f, 1.0f, 1.0f };
    vec3 leftColor = { 1.0f, 1.0f, 1.0f };
    vec3 rightColor = { 1.0f, 1.0f, 1.0f };
    vec3 backColor = { 1.0f, 1.0f, 1.0f };
    vec3 topColor = { 1.0f, 1.0f, 1.0f };
    vec3 bottomColor = { 1.0f, 1.0f, 1.0f };
};

using BlockTypes = std::vector<BlockType>;

static const std::vector<vec3> BlockFrontFace
    = { { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f } };
static const std::vector<vec3> BlockLeftFace
    = { { -0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, 0.5f } };
static const std::vector<vec3> BlockRightFace
    = { { 0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, -0.5f } };
static const std::vector<vec3> BlockBackFace
    = { { 0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f } };
static const std::vector<vec3> BlockTopFace
    = { { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f } };
static const std::vector<vec3> BlockBottomFace
    = { { -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f } };

} // namespace Game