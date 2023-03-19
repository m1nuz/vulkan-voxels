#include "ImageLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstring>

namespace ImageLoader {

auto load_image(const LoadImageInfo& info) -> std::optional<Image> {
    int width = 0, height = 0, channels = 0;
    const int req_comp = STBI_default; // STBI_rgb_alpha;
    auto ptr = stbi_load_from_memory(std::data(info.data), static_cast<int>(std::size(info.data)), &width, &height, &channels, req_comp);
    if (!ptr) {
        return std::nullopt;
    }

    Image img;
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.pixels.resize(width * height * channels);

    memcpy(std::data(img.pixels), ptr, std::size(img.pixels));

    stbi_image_free(ptr);

    return { img };
}

} // namespace ImageLoader