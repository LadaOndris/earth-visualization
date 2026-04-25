//
// Created by lada on 11/1/23.
//

#ifndef EARTH_VISUALIZATION_RESOURCEMANAGER_H
#define EARTH_VISUALIZATION_RESOURCEMANAGER_H

#include "textures/Texture.h"

#include <memory>
#include <list>
#include <algorithm>

class ResourceManager {
public:
    explicit ResourceManager(int maxTextures) : _maxTextures(maxTextures) {
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
        _loadedTextures++;
        _replacementQueue.push_front(texture);
    }

    /**
     * Moves the texture in the replacement queue to the beginning.
     */
    void noteUsage(const std::shared_ptr<Texture> &texture) {
        // Search for the texture in the replacement queue
        auto it = std::find(_replacementQueue.begin(), _replacementQueue.end(), texture);

        if (it != _replacementQueue.end()) {
            // If found, move it to the beginning of the queue (most recently used).
            _replacementQueue.splice(_replacementQueue.begin(), _replacementQueue, it);
        }
    }

    /**
     * Releases all loaded textures from the OpenGL context.
     */
    void releaseAll() {
        for (const auto &texture : _replacementQueue) {
            texture->unloadFromGL();
        }

        _replacementQueue.clear();
        _loadedTextures = 0;
    }

    [[nodiscard]] unsigned int getNumLoadedTextures() const {
        return _loadedTextures;
    }

private:
    int _maxTextures;
    int _loadedTextures = 0;
    std::list<std::shared_ptr<Texture>> _replacementQueue;

    /**
     * Decides whether a texture should be removed before a new one
     * is added.
     */
    [[nodiscard]] bool shouldReplaceTexture() const {
        return _loadedTextures >= _maxTextures;
    }

    /**
     * Removes a texture from the replacement queue.
     */
    void popTexture() {
        if (!_replacementQueue.empty()) {
            // Use LRU to remove the least recently used texture from the replacement queue.
            std::shared_ptr<Texture> textureToRemove = _replacementQueue.back();
            textureToRemove->unloadFromGL();
            _replacementQueue.pop_back();
            _loadedTextures--;
        }
    }
};


#endif //EARTH_VISUALIZATION_RESOURCEMANAGER_H
