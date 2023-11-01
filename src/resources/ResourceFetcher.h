//
// Created by lada on 10/31/23.
//

#ifndef EARTH_VISUALIZATION_RESOURCEFETCHER_H
#define EARTH_VISUALIZATION_RESOURCEFETCHER_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "../textures/Texture.h"

struct TextureLoadRequest {
    std::string path;
};

struct TextureLoadResult {
    std::string path;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> data;
};

extern std::queue<TextureLoadRequest> loadingTexturesQueue;
extern std::deque<TextureLoadResult> resultsQueue;
extern std::mutex loadingMutex;
extern std::mutex resultsMutex;
extern std::condition_variable cv;

class ResourceLoader {
public:
    [[noreturn]] void start() {
        while (true) {
            std::unique_lock<std::mutex> lock(loadingMutex);
            cv.wait(lock, [] { return !loadingTexturesQueue.empty(); });
            TextureLoadRequest request = loadingTexturesQueue.front();
            loadingTexturesQueue.pop();
            lock.unlock();

            // Load the texture data from file
            TextureLoadResult result;
            load(request, result);

            // Push the result to the results queue for the main thread
            {
                std::lock_guard<std::mutex> resultLock(resultsMutex);
                resultsQueue.push_back(result);
            }
        }
    }

    void load(const TextureLoadRequest &request, TextureLoadResult &result) {
        int width, height, channels;
        // stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(request.path.c_str(),
                                        &width, &height, &channels, 0);

        if (data) {
            result.path = request.path;
            result.width = width;
            result.height = height;
            result.channels = channels;
            result.data = std::vector<unsigned char>(data, data + width * height * channels);

            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture: " << request.path << std::endl;
        }
    }

};

class ResourceFetcher {
private:
public:
    void request(TextureLoadRequest job) {
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            loadingTexturesQueue.push(job);
        }
        cv.notify_one();
    }

    std::vector<TextureLoadResult> retrieveLoadedResources() {
        bool resultsAvailable = true;
        std::vector<TextureLoadResult> results;

        while (resultsAvailable) {
            // Check if results are available
            {
                std::lock_guard<std::mutex> resultLock(resultsMutex);
                resultsAvailable = !resultsQueue.empty();
            }

            if (resultsAvailable) {
                // If results are available, process them
                TextureLoadResult result;
                {
                    std::lock_guard<std::mutex> resultLock(resultsMutex);
                    result = resultsQueue.front();
                    resultsQueue.pop_front();

                }
                results.push_back(result);
            }
        }
        return results;
    }
};


#endif //EARTH_VISUALIZATION_RESOURCEFETCHER_H
