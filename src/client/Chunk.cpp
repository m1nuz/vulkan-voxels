#include "Chunk.hpp"

namespace Game {

auto create_chunk(const ivec2& position) -> Chunk {
    Chunk chunk;
    chunk._position = position;

    auto model = glm::mat4 { 1.0f };
    model = glm::translate(model, vec3(static_cast<float>(position.x * Chunk::Size), 0, static_cast<float>(position.y * Chunk::Size)));

    chunk._model = model;

    for (size_t y = 0; y < Chunk::Size; y++) {
        for (size_t x = 0; x < Chunk::Size; x++) {
            for (size_t z = 0; z < Chunk::Size; z++) {
                chunk._blocks[y][x][z] = 0;
            }
        }
    }

    return chunk;
}

auto build_chunk(Chunk& chunk, const BlockTypes& block_types) -> void {

    chunk._vertices.clear();
    chunk._indices.clear();
    chunk._vertices.reserve(chunk._vertex_count);
    chunk._indices.reserve(chunk._index_count);
    chunk._vertex_count = 0;
    chunk._index_count = 0;

    for (size_t y = 0; y < Chunk::Size; y++) {
        for (size_t x = 0; x < Chunk::Size; x++) {
            for (size_t z = 0; z < Chunk::Size; z++) {
                const auto block_index = chunk._blocks[y][x][z];
                if (block_index > 0) {
                    const auto block_type = block_types[block_index];
                    const auto translation = vec3 { x, y, z };

                    if (z == 0 || chunk._blocks[y][x][z - 1] == 0) {
                        chunk._vertices.push_back(
                            { BlockFrontFace[0] + translation, block_type.frontColor, vec3(0.0f, 1.0f, block_type.frontTexture) });
                        chunk._vertices.push_back(
                            { BlockFrontFace[1] + translation, block_type.frontColor, vec3(1.0f, 1.0f, block_type.frontTexture) });
                        chunk._vertices.push_back(
                            { BlockFrontFace[2] + translation, block_type.frontColor, vec3(1.0f, 0.0f, block_type.frontTexture) });
                        chunk._vertices.push_back(
                            { BlockFrontFace[3] + translation, block_type.frontColor, vec3(0.0f, 0.0f, block_type.frontTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }

                    if (x == 0 || chunk._blocks[y][x - 1][z] == 0) {
                        chunk._vertices.push_back(
                            { BlockLeftFace[0] + translation, block_type.leftColor, vec3(0.0f, 1.0f, block_type.leftTexture) });
                        chunk._vertices.push_back(
                            { BlockLeftFace[1] + translation, block_type.leftColor, vec3(1.0f, 1.0f, block_type.leftTexture) });
                        chunk._vertices.push_back(
                            { BlockLeftFace[2] + translation, block_type.leftColor, vec3(1.0f, 0.0f, block_type.leftTexture) });
                        chunk._vertices.push_back(
                            { BlockLeftFace[3] + translation, block_type.leftColor, vec3(0.0f, 0.0f, block_type.leftTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }

                    if (x == (Chunk::Size - 1) || chunk._blocks[y][x + 1][z] == 0) {
                        chunk._vertices.push_back(
                            { BlockRightFace[0] + translation, block_type.rightColor, vec3(0.0f, 1.0f, block_type.rightTexture) });
                        chunk._vertices.push_back(
                            { BlockRightFace[1] + translation, block_type.rightColor, vec3(1.0f, 1.0f, block_type.rightTexture) });
                        chunk._vertices.push_back(
                            { BlockRightFace[2] + translation, block_type.rightColor, vec3(1.0f, 0.0f, block_type.rightTexture) });
                        chunk._vertices.push_back(
                            { BlockRightFace[3] + translation, block_type.rightColor, vec3(0.0f, 0.0f, block_type.rightTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }

                    if (z == (Chunk::Size - 1) || chunk._blocks[y][x][z + 1] == 0) {
                        chunk._vertices.push_back(
                            { BlockBackFace[0] + translation, block_type.backColor, vec3(0.0f, 1.0f, block_type.backTexture) });
                        chunk._vertices.push_back(
                            { BlockBackFace[1] + translation, block_type.backColor, vec3(1.0f, 1.0f, block_type.backTexture) });
                        chunk._vertices.push_back(
                            { BlockBackFace[2] + translation, block_type.backColor, vec3(1.0f, 0.0f, block_type.backTexture) });
                        chunk._vertices.push_back(
                            { BlockBackFace[3] + translation, block_type.backColor, vec3(0.0f, 0.0f, block_type.backTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }

                    if (y == (Chunk::Size - 1) || chunk._blocks[y + 1][x][z] == 0) {
                        chunk._vertices.push_back(
                            { BlockTopFace[0] + translation, block_type.topColor, vec3(0.0f, 1.0f, block_type.topTexture) });
                        chunk._vertices.push_back(
                            { BlockTopFace[1] + translation, block_type.topColor, vec3(1.0f, 1.0f, block_type.topTexture) });
                        chunk._vertices.push_back(
                            { BlockTopFace[2] + translation, block_type.topColor, vec3(1.0f, 0.0f, block_type.topTexture) });
                        chunk._vertices.push_back(
                            { BlockTopFace[3] + translation, block_type.topColor, vec3(0.0f, 0.0f, block_type.topTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }

                    if (y == 0 || chunk._blocks[y - 1][x][z] == 0) {
                        chunk._vertices.push_back(
                            { BlockBottomFace[0] + translation, block_type.bottomColor, glm::vec3(0.0f, 1.0f, block_type.bottomTexture) });
                        chunk._vertices.push_back(
                            { BlockBottomFace[1] + translation, block_type.bottomColor, glm::vec3(1.0f, 1.0f, block_type.bottomTexture) });
                        chunk._vertices.push_back(
                            { BlockBottomFace[2] + translation, block_type.bottomColor, glm::vec3(1.0f, 0.0f, block_type.bottomTexture) });
                        chunk._vertices.push_back(
                            { BlockBottomFace[3] + translation, block_type.bottomColor, glm::vec3(0.0f, 0.0f, block_type.bottomTexture) });

                        chunk._indices.push_back(chunk._vertex_count);
                        chunk._indices.push_back(chunk._vertex_count + 1);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 2);
                        chunk._indices.push_back(chunk._vertex_count + 3);
                        chunk._indices.push_back(chunk._vertex_count);

                        chunk._vertex_count += 4;
                        chunk._index_count += 6;
                    }
                }
            }
        }
    }
}

} // namespace Game