
#pragma once


struct RenderingOptions {
    bool isSimulationRunning = false;
    bool isWireframeEnabled = false;
    bool isTextureEnabled = true;
    bool isNightEnabled = true;
    bool isTerrainEnabled = false;
    bool isTerrainShadingEnabled = true;
    bool isGridEnabled = false;
    bool isCullingEnabled = true;
    bool isRenderingCitiesEnabled = true;
    int simulationSpeed = 1;
    int heightFactor = 1000;
};
