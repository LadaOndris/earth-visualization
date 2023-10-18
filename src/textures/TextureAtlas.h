//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TEXTUREATLAS_H
#define EARTH_VISUALIZATION_TEXTUREATLAS_H

#include <utility>
#include <vector>
#include <string>
#include <cstring>
#include "Texture.h"
#include "../tiling/Tile.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <dirent.h>

class TextureAtlas {
private:
    std::vector<std::vector<std::vector<Texture>>> textures; // A 3D vector to store textures.

    Texture createTextureFromFile(std::string path) {
        Texture texture(std::move(path));
        return texture;
    }

    // A helper function to ensure vector dimensions match the given x and y size.
    void ensureVectorSize(std::vector<std::vector<Texture>> &vec, int x_size, int y_size) {
        if (vec.size() < x_size) {
            vec.resize(x_size);
        }
        for (auto &subvec: vec) {
            if (subvec.size() < y_size) {
                subvec.resize(y_size, Texture(""));
            }
        }
    }

    void registerTexturesFromSubdirectory(std::string levelDirPath, int level) {
        // Open the level directory.
        DIR *levelDir = opendir(levelDirPath.c_str());
        if (!levelDir) {
            std::cerr << "Failed to open level directory: " << levelDirPath << std::endl;
            return;
        }

        // Loop through files in the level directory.
        struct dirent *levelEntry;
        while ((levelEntry = readdir(levelDir))) {
            std::string fileName = levelEntry->d_name;
            if (fileName != "." && fileName != "..") {
                registerTextureFile(levelDirPath, fileName, level);
            }
        }

        closedir(levelDir);
    }

    void registerTextureFile(std::string levelDirPath, const std::string& fileName, int level) {
        std::string texturePath = std::move(levelDirPath);
        texturePath += "/";
        texturePath += fileName;

        // Parse the file name to extract relevant information.
        std::vector<std::string> tokens;
        std::istringstream tokenStream(fileName);
        std::string token;
        while (std::getline(tokenStream, token, '_')) {
            tokens.push_back(token);
        }

        if (tokens.size() == 7) {
            int x_index = std::atoi(tokens[1].c_str());
            int y_index = std::atoi(tokens[2].c_str());
            int x_tiles = std::atoi(tokens[3].c_str());
            int y_tiles = std::atoi(tokens[4].c_str());

            assert(x_index < x_tiles && x_tiles > 0);
            assert(y_index < y_tiles && y_tiles > 0);

            // Assuming you have a function to create a Texture instance from the file.
            Texture texture = createTextureFromFile(texturePath);

            // Ensure the vectors are appropriately sized.
            ensureVectorSize(textures[level], x_tiles, y_tiles);

            // Add the texture to the appropriate location in the textures vector.
            textures[level][x_index][y_index] = texture;
        }
    }

public:
    /**
     * Reads the directory for textures. It expects subfolders, each representing
     * a single level. The subfolders should contain images in the format
     * '{name}_{x_index}_{y_index}_{x_tiles}_{y_tiles}_{original_width}_{original_height}.png'
     *
     * @param path The directory containing tiles of all levels of detail.
     */
    void registerAvailableTextures(const std::string &path) {
        // This method should read the directory for textures and organize them into a 3D vector.

        // The textures 3D vector is organized as follows:
        // textures[level][x_index][y_index]

        // Level 0 is the most detailed level.
        // x_index and y_index represent the tile's position in longitude and latitude axes.

        // Open the directory for reading.
        DIR *directory = opendir(path.c_str());
        if (!directory) {
            std::cerr << "Failed to open directory: " << path << std::endl;
            return;
        }

        // Loop through files in the directory.
        int level = -1;
        struct dirent *entry;
        while ((entry = readdir(directory))) {
            // Check if the entry is a directory and skip ".", ".." entries.
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                std::string levelDirPath = path + "/" + entry->d_name;
                level++;

                // Create a new level in the textures vector if it doesn't exist.
                if (level >= static_cast<int>(textures.size())) {
                    textures.resize(level + 1);
                }

                registerTexturesFromSubdirectory(levelDirPath, level);
            }
        }
        closedir(directory);
    }

    /**
     * Returns the number of levels of detail available.
     */
    int getNumLevelsOfDetail() {
        return textures.size();
    }

    /**
     * Returns the number of tiles in both longitude and latitude axes for the most detailed level.
     */
    Resolution getMostDetailedLevelDimensions() {
        return getLevelDimensions(0);
    }

    /**
     * Returns the number of tiles in both longitude and latitude axes for the desired level.
     */
    Resolution getLevelDimensions(unsigned int level) {
        assert(level < textures.size());
        return Resolution(textures[level].size(), textures[level][0].size());
    }

    /**
     * Gets the texture, which correspond to the requested level of detail and tile.
     * @param level
     * @param tile
     * @return
     */
    Texture getTexture(unsigned int level, const Tile &tile) {
        if (level < textures.size() && tile.getLongitude() >= 0.0 && tile.getLatitude() >= 0.0) {
            int x_index = static_cast<int>(tile.getLongitude() / 360.0 * textures[level].size());
            int y_index = static_cast<int>(tile.getLatitude() / 180.0 * textures[level][0].size());

            if (x_index < textures[level].size() && y_index < textures[level][0].size()) {
                return textures[level][x_index][y_index];
            }
        }

        // Return a default empty texture if not found.
        return Texture("");
    }
};


#endif //EARTH_VISUALIZATION_TEXTUREATLAS_H
