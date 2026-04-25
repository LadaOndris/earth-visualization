#include "MeshBuffer.h"

#include <glad/glad.h>

namespace 
{

std::vector<t_vertex> convertToVertices(const std::vector<glm::vec3> &projectedVertices) {
    std::vector<t_vertex> convertedVertices;
    for (const auto &vec3: projectedVertices) {
        t_vertex vertex;
        vertex.x = vec3.x;
        vertex.y = vec3.y;
        vertex.z = vec3.z;

        convertedVertices.push_back(vertex);
    }
    return convertedVertices;
}

}

MeshBuffer::MeshBuffer(const Mesh_t& mesh)
    : _mesh(mesh) {
    const std::vector<t_vertex> vertices = convertToVertices(mesh);
    
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

const Mesh_t &MeshBuffer::getMesh() const {
    return _mesh;
}

unsigned int MeshBuffer::getVao() const {
    return _vao;
}
