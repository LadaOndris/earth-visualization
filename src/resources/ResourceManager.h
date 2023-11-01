//
// Created by lada on 11/1/23.
//

#ifndef EARTH_VISUALIZATION_RESOURCEMANAGER_H
#define EARTH_VISUALIZATION_RESOURCEMANAGER_H


#include <memory>
#include <list>
#include <algorithm>
#include "../textures/Texture.h"

class ResourceManager {
private:
    int maxTextures;
    int loadedTextures = 0;
    std::list<std::shared_ptr<Texture>> replacementQueue;

    /**
     * Decides whether a texture should be removed before a new one
     * is added.
     */
    bool shouldReplaceTexture() const {
        return loadedTextures >= maxTextures;
    }

    /**
     * Removes a texture from the replacement queue.
     */
    void popTexture() {
        if (!replacementQueue.empty()) {
            // Use LRU to remove the least recently used texture from the replacement queue.
            std::shared_ptr<Texture> textureToRemove = replacementQueue.back();
            textureToRemove->unloadFromGL();
            replacementQueue.pop_back();
            loadedTextures--;
        }
    }
public:
    explicit ResourceManager(int maxTextures) : maxTextures(maxTextures) {
    }

    /**
     * Loads the texture into the OpenGL context, potentially
     * removing an older, possibly unused texture.
     * @param texture
     */
    void addTextureIntoContext(const std::shared_ptr<Texture> &texture) {
        if (shouldReplaceTexture()) {
            popTexture();
        }
        texture->loadIntoGL();
        loadedTextures++;
        replacementQueue.push_front(texture);
    }

    /**
     * Moves the texture in the replacement queue to the beginning.
     */
    void noteUsage(const std::shared_ptr<Texture> &texture) {
        // Search for the texture in the replacement queue
        auto it = std::find(replacementQueue.begin(), replacementQueue.end(), texture);

        if (it != replacementQueue.end()) {
            // If found, move it to the beginning of the queue (most recently used).
            replacementQueue.splice(replacementQueue.begin(), replacementQueue, it);
        }
    }

    void releaseAll() {
        for (const auto &texture : replacementQueue) {
            texture->unloadFromGL();
        }

        replacementQueue.clear();
        loadedTextures = 0;
    }

    [[nodiscard]] unsigned int getNumLoadedTextures() const {
        return loadedTextures;
    }

};


#endif //EARTH_VISUALIZATION_RESOURCEMANAGER_H
