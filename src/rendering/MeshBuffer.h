#pragma once

#include "vertex.h"

#include <vector>

class MeshBuffer {
public:
    explicit MeshBuffer(const Mesh_t& vertices);
    ~MeshBuffer();

    MeshBuffer(const MeshBuffer&) = delete;
    MeshBuffer& operator=(const MeshBuffer&) = delete;

    const Mesh_t &getMesh() const;

    [[nodiscard]] unsigned int getVao() const;

private:
    Mesh_t _mesh;

    unsigned int _vao = 0;
    unsigned int _vbo = 0;
};
