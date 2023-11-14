
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include "shader.h"


class Program {
private:
    unsigned int id = 0;
    std::vector<std::unique_ptr<Shader>> shaders;

    [[nodiscard]] bool printErrorsIfAny() const {
        int success;
        char infoLog[512];
        // print linking errors if any
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            return true;
        }
        return false;
    }

    bool createProgram() {
        linkShaderProgram();

        bool errorsOccurred = printErrorsIfAny();
        if (errorsOccurred) {
            return false;
        }

        deleteShaders();
        return true;
    }

    void linkShaderProgram() {
        // program Program
        id = glCreateProgram();
        for (auto &shader : shaders) {
            glAttachShader(id, shader->getId());
        }
        glLinkProgram(id);
    }

    void deleteShaders() {
        // delete the shaders as they're linked into our program now and no longer necessary
        for (auto &shader : shaders) {
            shader->destroy();
        }
        shaders.clear();
    }

public:
    void addShader(std::unique_ptr<Shader> shader) {
        shaders.push_back(std::move(shader));
    }

    bool build() {
        for (auto &shader : shaders) {
            bool builtSucessfully = shader->build();

            if (!builtSucessfully) {
                return false;
            }
        }
        bool programCreated = createProgram();
        return programCreated;
    }

    // use/activate the program
    void use() const {
        assert(id != 0); // Assert the program has been built.
        glUseProgram(id);
    }

    // utility uniform functions
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int) value);
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

    ~Program() {
        deleteShaders();
    }
};

#endif