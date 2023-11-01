//
// Created by lada on 10/26/23.
//

#ifndef EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H
#define EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H

#include <glm/vec3.hpp>

struct RenderingStatistics {
    int frustumCulledTiles = 0;
    int backfacedCulledTiles = 0;
    int numTiles = 0;
    int loadedTextures = 0;
    glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
};

class RendererSubscriber {
public:
    virtual void notify(RenderingStatistics renderingStatistics) = 0;
};


#endif //EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H
