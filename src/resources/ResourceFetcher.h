//
// Created by lada on 10/31/23.
//

#ifndef EARTH_VISUALIZATION_RESOURCEFETCHER_H
#define EARTH_VISUALIZATION_RESOURCEFETCHER_H


#include <memory>
#include "../textures/Texture.h"

class ResourceFetcher {
private:
    int loadedTextures = 0;
    // Replacement queue
    // Request queue
public:
    void request(std::shared_ptr<Texture> texture) {
        if (!texture->isLoaded() && !texture->isPreparedInGlContext()) {
            texture->load();
            loadedTextures++;
        }
    }

    [[nodiscard]] unsigned int getNumLoadedTextures() const {
        return loadedTextures;
    }
};


#endif //EARTH_VISUALIZATION_RESOURCEFETCHER_H
