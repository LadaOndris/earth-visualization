

#ifndef EARTH_VISUALIZATION_PREFETCHER_H
#define EARTH_VISUALIZATION_PREFETCHER_H

#include "tiling/TileResources.h"

#include <vector>

class Prefetcher {
public:
    void prefetch(const std::vector<TileResources> &currentFrameResources) {
        // Implement resource prefetching logic based on the current frame's resources.
    }
};



#endif //EARTH_VISUALIZATION_PREFETCHER_H
