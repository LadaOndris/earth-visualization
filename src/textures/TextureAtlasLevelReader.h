
#pragma once

#include "TextureAtlasLevel.h"
#include "TextureFileReader.h"

#include <optional>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class TextureAtlasLevelReader {
public:
    std::optional<TextureAtlasLevel> read(const fs::path &levelDirPath) const {
        if (!fs::is_directory(levelDirPath)) {
            std::cerr << "Not a valid directory: " << levelDirPath << "\n";
            return std::nullopt;
        }

        std::optional<TextureAtlasLevel> atlasLevel;

        for (const auto &entry : fs::directory_iterator(levelDirPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            auto texture = _textureFileReader.read(levelDirPath, entry.path().filename());
            if (!texture) {
                std::cerr << "Failed to read texture file: " << entry.path() << "\n";
                continue;
            }
            if (!atlasLevel) {
                auto gridSize = (*texture)->getTextureGridSize();
                atlasLevel.emplace(gridSize.x, gridSize.y);
            }
            atlasLevel->addTexture(*texture);
        }

        return atlasLevel;
    }

private:
    TextureFileReader _textureFileReader;
};