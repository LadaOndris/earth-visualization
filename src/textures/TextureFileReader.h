
#pragma once

#include "Texture.h"

#include <optional>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

class TextureFileReader {
public:
    std::optional<std::shared_ptr<Texture>> read(const fs::path &levelDirPath, const fs::path &fileName) const;

private:
    struct TextureFileMetadata {
        int xIndex;
        int yIndex;
        int xTiles;
        int yTiles;
        int imageWidth;
    };

    std::vector<std::string> tokenizeFileName(const fs::path &fileName) const;
    std::optional<TextureFileMetadata> parseMetadata(const fs::path &fileName) const;
    std::shared_ptr<Texture> createTexture(const fs::path &texturePath, const TextureFileMetadata &meta) const;
};
