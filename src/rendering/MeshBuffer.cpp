#include "MeshBuffer.h"

#include <glad/glad.h>

MeshBuffer::MeshBuffer(const std::vector<t_vertex>& vertices) {
    glCreateBuffers(1, &_vbo);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glNamedBufferData(_vbo, vertices.size() * sizeof(t_vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

MeshBuffer::~MeshBuffer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}
