
#include <memory>
#include "gtest/gtest.h"
#include "../src/textures/TextureAtlas.h"

class TextureAtlasFixture : public ::testing::Test {
protected:
    virtual void SetUp() {
        textureAtlas = std::make_unique<TextureAtlas>();
        textureAtlas->registerAvailableTextures("textures/daymaps");
    }

    std::unique_ptr<TextureAtlas> textureAtlas;
};

std::string extractFileNameFromPath(const std::string& path) {
    size_t lastSlashPos = path.find_last_of('/');
    if (lastSlashPos != std::string::npos && lastSlashPos < path.length() - 1) {
        return path.substr(lastSlashPos + 1);
    } else {
        // If there's no slash in the path, return the path itself.
        return path;
    }
}

TEST_F(TextureAtlasFixture, DeterminesCorrectNumLevels) {
    auto numLevels = textureAtlas->getNumLevelsOfDetail();
    int expectedNumLevels = 2;

    ASSERT_EQ(numLevels, expectedNumLevels);
}

TEST_F(TextureAtlasFixture, CorrectDimensionsOfAllLevels) {
    auto level0Dims = textureAtlas->getLevelDimensions(0);
    EXPECT_EQ(level0Dims.getWidth(), 2);
    EXPECT_EQ(level0Dims.getHeight(), 1);

    auto level1Dims = textureAtlas->getLevelDimensions(1);
    EXPECT_EQ(level1Dims.getWidth(), 4);
    EXPECT_EQ(level1Dims.getHeight(), 2);
}

TEST_F(TextureAtlasFixture, TextureIsCorrectlyRegistered) {
    Tile tile(0, 90, 10, 10);
    auto texture = textureAtlas->getTexture(0, tile);

    bool isLoaded = texture->isLoaded();
    EXPECT_EQ(isLoaded, false);

    std::string path = texture->getPath();
    std::string fileName = extractFileNameFromPath(path);
    EXPECT_TRUE(strcmp(fileName.c_str(), "day_1_0_2_1_16200_8100.png"));
}