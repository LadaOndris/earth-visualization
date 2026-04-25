
#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

std::string loadFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Failed to open file: " + path.string());

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
}

enum ShaderType {
    Vertex,
    TesselationControl,
    TessellationEvaluation,
    Fragment
};

const char *shaderTypeToString(ShaderType e);

class Shader {
public:
    explicit Shader(const std::string& sourcePath, ShaderType type)
            : _sourcePath(sourcePath), _type(type) {
        build();
    }

    ~Shader() {
        if (_id != 0) {
            glDeleteShader(_id);
            _id = 0;
        }
    }

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept 
    : _sourcePath(std::move(other._sourcePath)), 
      _type(other._type),
      _id(other._id) {
        other._id = 0;
    }
    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (_id != 0)
                glDeleteShader(_id);

            _sourcePath = std::move(other._sourcePath);
            _type = other._type;
            _id = other._id;
            other._id = 0;
        }
        return *this;
    }

    [[nodiscard]] unsigned int getId() const {
        return _id;
    }

private:
    std::string _sourcePath;
    ShaderType _type;
    unsigned int _id = 0;

    void build() {
        std::string shaderSourceCode = loadFile(_sourcePath);
        const char* shaderSourceCodeCStr = shaderSourceCode.c_str();

        auto shaderTypeNumber = convertShaderTypeToGlNumber();
        _id = glCreateShader(shaderTypeNumber);
        glShaderSource(_id, 1, &shaderSourceCodeCStr, nullptr);
        glCompileShader(_id);

        checkErrors();
    }

    int convertShaderTypeToGlNumber() {
        switch (_type) {
            case Vertex:
                return GL_VERTEX_SHADER;
            case TesselationControl:
                return GL_TESS_CONTROL_SHADER;
            case TessellationEvaluation:
                return GL_TESS_EVALUATION_SHADER;
            case Fragment:
                return GL_FRAGMENT_SHADER;
        }
        throw std::runtime_error("Unsupported shader type");
    }

    void checkErrors() const {
        int success;
        char infoLog[512];

        glGetShaderiv(_id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(_id, 512, nullptr, infoLog);
            throw std::runtime_error("Failed to compile shader: " + _sourcePath);
        }
    }

};
