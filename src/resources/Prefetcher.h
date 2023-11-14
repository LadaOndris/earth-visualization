//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_PREFETCHER_H
#define EARTH_VISUALIZATION_PREFETCHER_H

#include <vector>
#include "../tiling/TileResources.h"

class Prefetcher {
public:
    void prefetch(const std::vector<TileResources> &currentFrameResources) {
        // Implement resource prefetching logic based on the current frame's resources.
    }
};



#endif //EARTH_VISUALIZATION_PREFETCHER_H
