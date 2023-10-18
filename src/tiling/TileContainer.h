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

            Resolution meshResolution = determineMeshResolution(heightMap, tile);
            // The ellipsoid is used to project the mesh onto it.
            // The tile determines the position of the mesh on the ellipsoid.
            Mesh_t mesh = tileMeshTesselator.generate(meshResolution, ellipsoid, tile);
            TileResources tileResource(mesh, colorMap, heightMap);

            tile.addResources(tileResource);
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
        assert(heightMap.isLoaded());

        // Get the dimensions of the height map texture.
        int textureWidth = heightMap.getResolution().getWidth();
        int textureHeight = heightMap.getResolution().getHeight();

        // Calculate the portion of the texture that corresponds to the given tile.
        double textureXStart = (tile.getLongitude() + 180) / 360.0;
        double textureYStart = (tile.getLatitude() + 90) / 180.0;
        double textureXEnd = (tile.getLongitude() + tile.getLongitudeWidth() + 180) / 360.0;
        double textureYEnd = (tile.getLatitude() + tile.getLatitudeWidth() + 90) / 180.0;

        // Calculate the width and height of the texture portion.
        int texturePortionWidth = static_cast<int>(textureWidth * (textureXEnd - textureXStart));
        int texturePortionHeight = static_cast<int>(textureHeight * (textureYEnd - textureYStart));

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

    std::vector<Tile> getTiles() {
        return tiles;
    }
};

#endif //EARTH_VISUALIZATION_TILECONTAINER_H
