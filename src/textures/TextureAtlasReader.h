
#pragma once

#include "TextureAtlasLevel.h"
#include "TextureAtlasLevelReader.h"

#include <algorithm>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class TextureAtlasReader {
public:
    std::optional<std::vector<TextureAtlasLevel>> read(const fs::path &path) const {
        if (!fs::is_directory(path)) {
            std::cerr << "Not a valid directory: " << path << "\n";
            return std::nullopt;
        }

        auto levelDirs = collectLevelDirectories(path);
        if (!levelDirs || levelDirs->empty()) {
            std::cout << "No level directories found in " << path << "\n";
            return std::nullopt;
        }

        if (!validateNoLevelsSkipped(*levelDirs)) {
            return std::nullopt;
        }

        return readLevels(*levelDirs);
    }

private:
    TextureAtlasLevelReader _levelReader;

    std::optional<int> parseLevelFromDirName(const std::string &dirName) const {
        if (!dirName.starts_with("level_")) {
            return std::nullopt;
        }
        const std::string levelStr = dirName.substr(6);
        if (levelStr.empty() || !std::ranges::all_of(levelStr, ::isdigit)) {
            return std::nullopt;
        }
        return std::stoi(levelStr);
    }

    std::optional<std::map<int, fs::path>> collectLevelDirectories(const fs::path &path) const {
        std::map<int, fs::path> levelDirs;

        for (const auto &entry : fs::directory_iterator(path)) {
            if (!entry.is_directory()) {
                continue;
            }
            const std::string dirName = entry.path().filename().string();
            auto level = parseLevelFromDirName(dirName);
            if (!level) {
                std::cerr << "Unexpected directory format (expected 'level_<N>'): " << dirName << "\n";
                return std::nullopt;
            }
            if (levelDirs.contains(*level)) {
                std::cerr << "Duplicate level directory for level " << *level << "\n";
                return std::nullopt;
            }
            levelDirs[*level] = entry.path();
        }

        return levelDirs;
    }

    bool validateNoLevelsSkipped(const std::map<int, fs::path> &levelDirs) const {
        const int minLevel = levelDirs.begin()->first;
        const int maxLevel = levelDirs.rbegin()->first;
        for (int level = minLevel; level <= maxLevel; ++level) {
            if (!levelDirs.contains(level)) {
                std::cerr << "Missing level directory for level " << level << "\n";
                return false;
            }
        }
        return true;
    }

    std::vector<TextureAtlasLevel> readLevels(const std::map<int, fs::path> &levelDirs) const {
        std::vector<TextureAtlasLevel> levels;
        levels.reserve(levelDirs.size());

        for (const auto &[level, dirPath] : levelDirs) {
            auto atlasLevel = _levelReader.read(dirPath);
            if (!atlasLevel) {
                std::cerr << "Failed to read level " << level << " from " << dirPath << "\n";
                continue;
            }
            levels.push_back(std::move(*atlasLevel));
        }

        return levels;
    }
};