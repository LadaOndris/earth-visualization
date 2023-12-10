//
// Created by lada on 10/25/23.
//

#ifndef EARTH_VISUALIZATION_RENDERINGOPTIONS_H
#define EARTH_VISUALIZATION_RENDERINGOPTIONS_H


struct RenderingOptions {
    bool isSimulationRunning = false;
    bool isWireframeEnabled = false;
    bool isTextureEnabled = true;
    bool isNightEnabled = true;
    bool isTerrainEnabled = false;
    bool isTerrainShadingEnabled = true;
    bool isGridEnabled = false;
    bool isCullingEnabled = true;
    int simulationSpeed = 1;
    int heightFactor = 1000;
};


#endif //EARTH_VISUALIZATION_RENDERINGOPTIONS_H
