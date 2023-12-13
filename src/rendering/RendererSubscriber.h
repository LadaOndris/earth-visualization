//
// Created by lada on 10/26/23.
//

#ifndef EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H
#define EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct RenderingStatistics {
    unsigned int frustumCulledTiles = 0;
    unsigned int backfacedCulledTiles = 0;
    unsigned int numTiles = 0;
    unsigned int loadedTextures = 0;
    glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
    glm::vec2 renderedLatitudeRange = glm::vec2(0, 0);
    glm::vec2 renderedLongitudeRange = glm::vec2(0, 0);
};

class RendererSubscriber {
public:
    virtual void notify(RenderingStatistics renderingStatistics) = 0;
};


#endif //EARTH_VISUALIZATION_RENDERERSUBSCRIBER_H
