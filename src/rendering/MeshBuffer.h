#pragma once

#include "vertex.h"

#include <vector>

class MeshBuffer {
public:
    explicit MeshBuffer(const std::vector<t_vertex>& vertices);
    ~MeshBuffer();

    MeshBuffer(const MeshBuffer&) = delete;
    MeshBuffer& operator=(const MeshBuffer&) = delete;

    [[nodiscard]] unsigned int vao() const { return _vao; }

private:
    unsigned int _vao = 0;
    unsigned int _vbo = 0;
};
