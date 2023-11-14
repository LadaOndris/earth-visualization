
#include "../../include/shader.h"

const char *shaderTypeToString(ShaderType e) {
    switch (e) {
        case ShaderType::Vertex:
            return "Vertex";
        case ShaderType::TesselationControl:
            return "TesselationControl";
        case ShaderType::TessellationEvaluation:
            return "TessellationEvaluation";
        case ShaderType::Fragment:
            return "Fragment";
        default:
            throw std::invalid_argument("Unimplemented item");
    }
}