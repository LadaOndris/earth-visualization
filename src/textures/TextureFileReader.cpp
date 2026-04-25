
#include "TextureFileReader.h"

#include <sstream>
#include <iostream>
#include <glm/vec2.hpp>

std::optional<std::shared_ptr<Texture>> TextureFileReader::read(const fs::path &levelDirPath, const fs::path &fileName) const {
    const auto meta = parseMetadata(fileName);
    if (!meta) {
        return std::nullopt;
    }
    return createTexture(levelDirPath / fileName, *meta);
}

std::vector<std::string> TextureFileReader::tokenizeFileName(const fs::path &fileName) const {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(fileName.string());
    std::string token;
    while (std::getline(tokenStream, token, '_')) {
        tokens.push_back(token);
    }
    return tokens;
}

std::optional<TextureFileReader::TextureFileMetadata> TextureFileReader::parseMetadata(const fs::path &fileName) const {
    const auto tokens = tokenizeFileName(fileName);
    if (tokens.size() != 8) {
        std::cerr << "Unexpected filename format: " << fileName << "\n";
        return std::nullopt;
    }

    TextureFileMetadata meta {
        .xIndex     = std::stoi(tokens[1]),
        .yIndex     = std::stoi(tokens[2]),
        .xTiles     = std::stoi(tokens[3]),
        .yTiles     = std::stoi(tokens[4]),
        .imageWidth = std::stoi(tokens[7]),
    };

    if (meta.xTiles <= 0 || meta.yTiles <= 0 || meta.imageWidth <= 0) {
        std::cerr << "Invalid tile dimensions in filename: " << fileName << "\n";
        return std::nullopt;
    }
    if (meta.xIndex >= meta.xTiles || meta.yIndex >= meta.yTiles) {
        std::cerr << "Tile index out of bounds in filename: " << fileName << "\n";
        return std::nullopt;
    }

    return meta;
}

std::shared_ptr<Texture> TextureFileReader::createTexture(const fs::path &texturePath, const TextureFileMetadata &meta) const {
    const double longitudeOffset = static_cast<double>(meta.xIndex) / meta.xTiles * 360.0 - 180.0;
    const double latitudeOffset  = static_cast<double>(meta.yIndex) / meta.yTiles * 180.0 - 90.0;
    const double longitudeWidth  = 1.0 / meta.xTiles * 360.0;
    const double latitudeWidth   = 1.0 / meta.yTiles * 180.0;

    return std::make_shared<Texture>(
        texturePath,
        meta.imageWidth,
        glm::vec2(longitudeOffset, latitudeOffset),
        glm::vec2(longitudeWidth,  latitudeWidth),
        glm::vec2(meta.xTiles,     meta.yTiles),
        meta.xIndex,
        meta.yIndex
    );
}
