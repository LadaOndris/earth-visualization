
#pragma once

#include "program.h"
#include "shader.h"

#include <vector>
#include <memory>

class ProgramBuilder {
private:
    std::vector<std::unique_ptr<Shader>> shaders;

public:
    ProgramBuilder() = default;

    ProgramBuilder& addShader(const std::string& shaderPath, ShaderType shaderType) {
        shaders.push_back(std::make_unique<Shader>(shaderPath, shaderType));
        return *this;
    }

    Program build() {
        return Program(std::move(shaders));
    }
};