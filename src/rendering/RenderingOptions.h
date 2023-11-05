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
    bool isGridEnabled = false;
    int simulationSpeed = 1;
};


#endif //EARTH_VISUALIZATION_RENDERINGOPTIONS_H
