#pragma once

#include "Block.hpp"
#include "Math.hpp"
#include "Vertex.hpp"

#include <vector>

namespace Game {

struct Chunk {
    using Vertices = std::vector<Vertex>;
    using Indices = std::vector<uint32_t>;

    static constexpr size_t Size = 64;

    ivec2 _position = ivec2 { 0, 0 };
    mat4 _model;

    uint32_t _blocks[Size][Size][Size];

    size_t _vertex_count = 0;
    size_t _index_count = 0;

    Vertices _vertices;
    Indices _indices;
};

auto create_chunk(const ivec2& position) -> Chunk;
auto build_chunk(Chunk& chunk, const BlockTypes& block_types) -> void;

} // namespace Game