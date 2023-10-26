//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILECONTAINER_H
#define EARTH_VISUALIZATION_TILECONTAINER_H


#include <vector>
#include <string>
#include <cassert>
#include "../textures/Texture.h"
#include "../textures/TextureAtlas.h"
#include "../tesselation/TileMeshTesselator.h"


// Define the TileContainer class.

class TileContainer {
private:
    std::vector<Tile> tiles;
    TileMeshTesselator &tileMeshTesselator;
    TextureAtlas &colorMapAtlas;
    TextureAtlas &heightMapAtlas;
    Ellipsoid &ellipsoid;
    std::vector<Mesh_t> cachedMeshes;

    /**
    * Assigns the corresponding resources from texture atlases to
    * the given tile. Generates meshes for all supported levels of detail.
    */
    void setupTile(Tile &tile) {
        int numLevels = heightMapAtlas.getNumLevelsOfDetail();

        for (int level = 0; level < numLevels; ++level) {
            // Based on the information of the tile and the current level,
            // the texture atlas returns the correct texture
            Texture heightMap = heightMapAtlas.getTexture(level, tile);
            Texture colorMap = colorMapAtlas.getTexture(level, tile);

            if (level >= cachedMeshes.size()) {
                // The heightMap determines the resolution of the mesh.
                // Altough, the resolution of each heightmap image is the same,
                // the area it covers differs. Thus, the resolution of the
                // resulting mesh will also differ.
                Resolution meshResolution = determineMeshResolution(heightMap, tile);
                std::cout << meshResolution.getWidth() << ", " << meshResolution.getHeight() << std::endl;
                // The ellipsoid is used to project the mesh onto it.
                // The tile determines the position of the mesh on the ellipsoid.
                Mesh_t mesh = tileMeshTesselator.generate(meshResolution, tile);

                std::cout << "Mesh size (triangles): " << mesh.size() / 3 << std::endl;
                cachedMeshes.push_back(mesh);
            }

            Mesh_t mesh = cachedMeshes[level];
            auto tileResource = std::make_shared<TileResources>(mesh, colorMap, heightMap);

            tile.addResources(tileResource, level);
        }
    }

    /**
    * Determine the mesh resolution for a given tile based on the height map texture and the
    * portion of the ellipsoid it covers.
    * The goal is to calculate the resolution such that each pixel in the texture corresponds
    * to a vertex in the mesh.
    *
    * @param heightMap The height map texture that provides elevation data.
    * @param tile The tile representing the portion of the ellipsoid covered by the mesh.
    *
    * @return A Resolution object representing the width and height of the mesh, matching the
    * dimensions of the texture portion.
    */
    Resolution determineMeshResolution(const Texture &heightMap, const Tile &tile) {
        double makeSmallerCoefficient = 1.0 / 10;

        // Get the dimensions of the height map texture.
        int textureWidth = heightMap.getResolution().getWidth();
        int textureHeight = heightMap.getResolution().getHeight();
        assert(textureWidth > 0 && textureHeight > 0);

        // Calculate the portion of the tile in the given texture
        double tileWidthPortion = tile.getLongitudeWidth() / heightMap.getLongitudeWidth();
        double tileHeightPortion = tile.getLatitudeWidth() / heightMap.getLatitudeWidth();

        // Calculate the width and height of the texture portion.
        int texturePortionWidth = static_cast<int>(textureWidth * tileWidthPortion * makeSmallerCoefficient);
        int texturePortionHeight = static_cast<int>(textureHeight * tileHeightPortion * makeSmallerCoefficient);

        // The mesh resolution should match the texture portion dimensions.
        return Resolution(texturePortionWidth, texturePortionHeight);
    }

    /**
     * Implement globe division into tiles based on the most detailed resolution.
     * The resolution determines how many tiles there will be per longitude and latitude.
     * Even the coarse levels of detail will be composed of the same number of tiles. They
     * will be simply composed of a smaller number of triangles and show a different texture.
     *
     * @param numTilesLongitude Total tiles along the longitude (360 degrees)
     * @param numTilesLatitude Total tiles along the latitude (180 degrees)
     */
    void divideGlobeIntoTiles(int numTilesLongitude, int numTilesLatitude) {
        // Calculate the width and height of each tile in degrees.
        double tileWidth = 360.0 / numTilesLongitude;
        double tileHeight = 180.0 / numTilesLatitude;

        for (int latIndex = 0; latIndex < numTilesLatitude; ++latIndex) {
            for (int lonIndex = 0; lonIndex < numTilesLongitude; ++lonIndex) {
                double latitude = latIndex * tileHeight - 90.0;
                double longitude = lonIndex * tileWidth - 180.0;

                Tile tile(latitude, longitude, tileHeight, tileWidth);
                tiles.push_back(tile);
            }
        }
    }

public:
    explicit TileContainer(TileMeshTesselator &tileMeshTesselator,
                           TextureAtlas &colorMapAtlas,
                           TextureAtlas &heightMapAtlas,
                           Ellipsoid &ellipsoid)
            : tileMeshTesselator(tileMeshTesselator),
              colorMapAtlas(colorMapAtlas),
              heightMapAtlas(heightMapAtlas),
              ellipsoid(ellipsoid) {

    }

    /**
    * Initializes the resources needed for rendering. Texture atlases are
    * used to load the textures at the various available levels of detail,
    * and divides the globe into tiles based on the most detailed level of detail.
    */
    void setupTiles() {
        assert(colorMapAtlas.getNumLevelsOfDetail() == heightMapAtlas.getNumLevelsOfDetail());

        // Needs to know the finest heightMap resolution
        Resolution dimensions = heightMapAtlas.getMostDetailedLevelDimensions();
        this->divideGlobeIntoTiles(dimensions.getWidth(), dimensions.getHeight());

        for (Tile &tile: tiles) {
            setupTile(tile);
        }
    }

    std::vector<Tile> &getTiles() {
        return tiles;
    }

    int getNumLevels() const {
        return colorMapAtlas.getNumLevelsOfDetail();
    }
};

#endif //EARTH_VISUALIZATION_TILECONTAINER_H
