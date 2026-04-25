
#pragma once

#include "shader.h"

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <memory>
#include <stdexcept>
#include <vector>


class Program {
public:
    explicit Program(std::vector<std::unique_ptr<Shader>> shaders) {
        if (shaders.empty())
            throw std::runtime_error("Program requires at least one shader");

        id = glCreateProgram();

        for (auto& shader : shaders) {
            glAttachShader(id, shader->getId());
        }

        glLinkProgram(id);
        
        checkLinkErrors(id);
    }

    ~Program() {
        if (id != 0)
            glDeleteProgram(id);
    }

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    Program(Program&& other) noexcept : id(other.id) {
        other.id = 0;
    }

    Program& operator=(Program&& other) noexcept {
        if (this != &other) {
            if (id != 0)
                glDeleteProgram(id);

            id = other.id;
            other.id = 0;
        }
        return *this;
    }
    
    void use() const {
        assert(id != 0);
        glUseProgram(id);
    }

    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    void setVec3(const std::string &name, glm::vec3 vec) const {
        int location = glGetUniformLocation(id, name.c_str());
        glUniform3fv(location, 1, glm::value_ptr(vec));
    }

    void setVec2(const std::string &name, glm::vec2 vec) const {
        int location = glGetUniformLocation(id, name.c_str());
        glUniform2fv(location, 1, glm::value_ptr(vec));
    }

    void setMat4(const std::string &name, glm::mat4 matrix) const {
        int matrixLocation = glGetUniformLocation(id, name.c_str());
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
    }

private:
    unsigned int id = 0;
    std::vector<std::unique_ptr<Shader>> _shaders;

    static void checkLinkErrors(unsigned int programId) {
        int success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);

        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(programId, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("Program linking failed:\n") + infoLog);
        }
    }
};