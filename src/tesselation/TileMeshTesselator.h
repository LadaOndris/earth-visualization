//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILEMESHTESSELATOR_H
#define EARTH_VISUALIZATION_TILEMESHTESSELATOR_H

#include <vector>
#include "../textures/Texture.h"
#include "../ellipsoid.h"
#include "../tiling/Tile.h"

class TileMeshTesselator {
private:
    /**
     * Calculate the 3D position of a vertex in the mesh based on its grid position, resolution, and ellipsoid.
     *
     * This function projects a grid point onto the ellipsoid's surface to determine the vertex's position.
     *
     * @param x The x-coordinate of the grid point.
     * @param y The y-coordinate of the grid point.
     * @param meshResolution The resolution of the mesh (width and height).
     * @param ellipsoid The ellipsoid onto which the mesh is projected.
     * @param tile The part of the ellipsoid being represented.
     * @return The 3D position of the vertex on the ellipsoid's surface.
     */
    glm::vec3 calculateVertexPosition(int x, int y, const Resolution& meshResolution,
                                      const Ellipsoid& ellipsoid, const Tile& tile) {
        double tileLongitudeStart = tile.getLongitude();
        double tileLongitudeEnd = tile.getLongitude() + tile.getLongitudeWidth();
        double tileLatitudeStart = tile.getLatitude();
        double tileLatitudeEnd = tile.getLatitude() + tile.getLatitudeWidth();

        // Adjust this logic to consider the ellipsoid's shape and projection method.
        double longitude = tileLongitudeStart + (x / static_cast<double>(meshResolution.getWidth())) * (tileLongitudeEnd - tileLongitudeStart);
        double latitude = tileLatitudeStart + (y / static_cast<double>(meshResolution.getHeight())) * (tileLatitudeEnd - tileLatitudeStart);

        // Project the latitude and longitude to 3D coordinates on the ellipsoid's surface.
        glm::vec3 geodeticCoordinates(longitude, latitude, 0.0);  // Adjust height as needed.
        glm::vec3 surfaceNormal = ellipsoid.geodeticSurfaceNormalFromGeodetic(geodeticCoordinates);
        glm::vec3 vertexPosition = ellipsoid.projectPointOntoSurface(surfaceNormal);  // Adjust as per your ellipsoid.

        return vertexPosition;
    }
public:
    /**
     * Generate a uniform triangle mesh for a given tile with the specified resolution.
     *
     * This function creates a mesh of triangles to cover a tile's geographical region on the ellipsoid's surface.
     *
     * @param meshResolution The resolution (width and height) of the generated uniform mesh.
     * @param ellipsoid The ellipsoid onto which the mesh is projected.
     * @param tile The specific part of the ellipsoid that the mesh represents.
     * @return A vector of 3D vertices forming triangles. Each group of three vertices defines a single triangle.
     */
    std::vector<glm::vec3> generate(Resolution &meshResolution, Ellipsoid &ellipsoid, const Tile &tile) {

        std::vector<glm::vec3> meshVertices;

        // You should generate mesh vertices and populate the meshVertices vector.

        // Example: Generate a simple quad mesh (adjust for your actual mesh generation):
        for (int y = 0; y < meshResolution.getHeight() - 1; y++) {
            for (int x = 0; x < meshResolution.getWidth() - 1; x++) {
                // Calculate vertex positions based on resolution, ellipsoid, and tile information.
                glm::vec3 vertex1 = calculateVertexPosition(x, y, meshResolution, ellipsoid, tile);
                glm::vec3 vertex2 = calculateVertexPosition(x + 1, y, meshResolution, ellipsoid, tile);
                glm::vec3 vertex3 = calculateVertexPosition(x, y + 1, meshResolution, ellipsoid, tile);
                glm::vec3 vertex4 = calculateVertexPosition(x + 1, y + 1, meshResolution, ellipsoid, tile);

                // Add vertices to the meshVertices vector, forming triangles.
                meshVertices.push_back(vertex1);
                meshVertices.push_back(vertex2);
                meshVertices.push_back(vertex3);

                meshVertices.push_back(vertex2);
                meshVertices.push_back(vertex4);
                meshVertices.push_back(vertex3);
            }
        }

        return meshVertices;
    }
};


#endif //EARTH_VISUALIZATION_TILEMESHTESSELATOR_H
