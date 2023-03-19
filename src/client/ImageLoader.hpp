#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace ImageLoader {

struct Image {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint32_t channels = 0;
    std::vector<uint8_t> pixels;
};

struct LoadImageInfo {
    std::span<uint8_t> data;
    // std::vector<uint8_t> data;
};

auto load_image(const LoadImageInfo& info) -> std::optional<Image>;

} // namespace ImageLoader