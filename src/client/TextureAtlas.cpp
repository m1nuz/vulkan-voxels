#include "TextureAtlas.hpp"
#include "Content.hpp"
#include "ImageLoader.hpp"
#include "Journal.hpp"
#include "Json.hpp"
#include "Tags.hpp"

namespace Graphics {

using ByteBuffer = std::vector<uint8_t>;

auto append_texture(TextureAtlas& atlas, std::string_view name, std::string_view filepath) -> bool {

    auto content = Content::read<ByteBuffer>(filepath);
    if (!content) {
        Journal::error(Tags::Graphics, "Failed to load '{}'", filepath);
        return false;
    }

    auto image = ImageLoader::load_image({ .data = *content });
    if (!image) {
        Journal::error(Tags::Graphics, "Failed to read '{}'", name);
        return false;
    }

    TextureInfo texture_info;
    texture_info.width = image->width;
    texture_info.height = image->height;
    texture_info.channels = image->channels;
    texture_info.pixels = image->pixels;
    texture_info.name = name;
    texture_info.filepath = filepath;

    Journal::debug(Tags::Graphics, "Image '{}' {}x{} {}", name, image->width, image->height, image->channels * 8);

    atlas._textures.push_back(texture_info);

    return true;
}

auto get_texture_atlas([[maybe_unused]] std::string_view info) -> TextureAtlas {
    TextureAtlas atlas;

    auto j = Json::parse(std::begin(info), std::end(info));

    if (j.find("textures") != std::end(j)) {
        for (const auto& t : j["textures"]) {
            const auto texture_filename = value_or_default(t, "name", std::string { "blank" });
            const auto texture_filepath = value_or_default(t, "file", std::string { "textures/blank.png" });

            append_texture(atlas, texture_filename, "../assets/" + texture_filepath);
        }
    }

    return atlas;
}

} // namespace Graphics