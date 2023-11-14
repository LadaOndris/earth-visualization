//
// Created by lada on 11/10/23.
//

#ifndef EARTH_VISUALIZATION_SHADER_H
#define EARTH_VISUALIZATION_SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

enum ShaderType {
    Vertex,
    TesselationControl,
    TessellationEvaluation,
    Fragment
};

const char *shaderTypeToString(ShaderType e);

class Shader {
private:
    const char *sourcePath;
    const ShaderType type;
    unsigned int id = 0;


    [[nodiscard]] bool printErrorsIfAny() const {
        int success;
        char infoLog[512];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::" << shaderTypeToString(type) << "::COMPILATION_FAILED\n" << infoLog
                      << std::endl;
            return true;
        }
        return false;
    }

public:
    explicit Shader(const char *sourcePath, ShaderType type)
            : sourcePath(sourcePath), type(type) {

    }

    bool build() {
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            shaderFile.open(sourcePath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << shaderFile.rdbuf();
            // close file handlers
            shaderFile.close();
            // convert stream into string
            shaderCode = vShaderStream.str();
        }
        catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            return false;
        }
        const char *shaderSource = shaderCode.c_str();

        auto shaderTypeNumber = convertShaderTypeToGlNumber();
        id = glCreateShader(shaderTypeNumber);
        glShaderSource(id, 1, &shaderSource, nullptr);
        glCompileShader(id);

        bool errorsOccurred = printErrorsIfAny();
        if (errorsOccurred) {
            return false;
        }
        return true;
    }

    int convertShaderTypeToGlNumber() {
        switch (type) {
            case Vertex:
                return GL_VERTEX_SHADER;
            case TesselationControl:
                return GL_TESS_CONTROL_SHADER;
            case TessellationEvaluation:
                return GL_TESS_EVALUATION_SHADER;
            case Fragment:
                return GL_FRAGMENT_SHADER;
        }
        throw std::runtime_error("Unssuported shader type");
    }

    [[nodiscard]] unsigned int getId() const {
        return id;
    }

    void destroy() {
        // No need to delete if uninitialized
        if (id == 0) {
            return;
        }
        glDeleteShader(id);
        id = 0;
    }
};

#endif //EARTH_VISUALIZATION_SHADER_H
