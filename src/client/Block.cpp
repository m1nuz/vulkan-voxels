#include "Block.hpp"
#include "Journal.hpp"
#include "Json.hpp"
#include "Tags.hpp"

namespace Game {

auto get_block_types(std::string_view info) -> BlockTypes {
    BlockTypes block_types;

    auto j = Json::parse(std::begin(info), std::end(info));

    if (j.find("block_types") != std::end(j)) {
        for (const auto& bt : j["block_types"]) {
            BlockType block_type;
            block_type.frontTexture = bt["front"]["texture"];
            block_type.frontColor = vec3 { bt["front"]["color"][0], bt["front"]["color"][1], bt["front"]["color"][2] };

            block_type.leftTexture = bt["left"]["texture"];
            block_type.leftColor = vec3 { bt["left"]["color"][0], bt["left"]["color"][1], bt["left"]["color"][2] };

            block_type.rightTexture = bt["right"]["texture"];
            block_type.rightColor = vec3 { bt["right"]["color"][0], bt["right"]["color"][1], bt["right"]["color"][2] };

            block_type.backTexture = bt["back"]["texture"];
            block_type.backColor = vec3 { bt["back"]["color"][0], bt["back"]["color"][1], bt["back"]["color"][2] };

            block_type.topTexture = bt["top"]["texture"];
            block_type.topColor = vec3 { bt["top"]["color"][0], bt["top"]["color"][1], bt["top"]["color"][2] };

            block_type.bottomTexture = bt["bottom"]["texture"];
            block_type.bottomColor = vec3 { bt["bottom"]["color"][0], bt["bottom"]["color"][1], bt["bottom"]["color"][2] };

            Journal::debug(Tags::Game, "Block {} {} {} {} {} {}", block_type.frontTexture, block_type.leftTexture, block_type.rightTexture,
                block_type.backTexture, block_type.topTexture, block_type.bottomTexture);

            block_types.push_back(block_type);
        }
    }

    if (block_types.empty()) {
        Journal::warning(Tags::Game, "{}", "Info about blocks not found!");
    }

    return block_types;
}

} // namespace Game