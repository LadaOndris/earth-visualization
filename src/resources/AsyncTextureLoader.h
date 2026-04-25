#pragma once

#include "textures/Texture.h"

#include <iostream>
#include <queue>
#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>

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

class AsyncTextureLoader {
public:
    AsyncTextureLoader() {
        worker = std::thread(&AsyncTextureLoader::workerLoop, this);
    }

    ~AsyncTextureLoader() {
        stop();
    }

    AsyncTextureLoader(const AsyncTextureLoader&) = delete;
    AsyncTextureLoader& operator=(const AsyncTextureLoader&) = delete;

    void request(TextureLoadRequest request) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            requests.push(std::move(request));
        }
        cvRequestMade.notify_one();
    }

    std::vector<TextureLoadResult> getResults() {
        std::vector<TextureLoadResult> out;

        std::lock_guard<std::mutex> lock(mutex);
        while (!results.empty()) {
            out.push_back(std::move(results.front()));
            results.pop_front();
        }

        return out;
    }

    void stop() {
        bool expected = false;
        if (!stopped.compare_exchange_strong(expected, true)) {
            return;
        }

        stopFlag = true;
        cvRequestMade.notify_all();

        if (worker.joinable()) {
            worker.join();
        }
    }

private:
    void workerLoop() {
        while (true) {
            TextureLoadRequest request;

            {
                std::unique_lock<std::mutex> lock(mutex);

                cvRequestMade.wait(lock, [this] {
                    return stopFlag || !requests.empty();
                });

                if (stopFlag && requests.empty()) {
                    return;
                }

                request = std::move(requests.front());
                requests.pop();
            }

            TextureLoadResult result = load(request);

            {
                std::lock_guard<std::mutex> lock(mutex);
                results.push_back(std::move(result));
            }
        }
    }

    TextureLoadResult load(const TextureLoadRequest& request) {
        TextureLoadResult result;

        int width, height, channels;
        unsigned char* data = stbi_load(request.path.c_str(),
                                        &width, &height, &channels, 0);

        if (data) {
            result.path = request.path;
            result.width = width;
            result.height = height;
            result.channels = channels;
            result.data = std::vector<unsigned char>(
                data, data + width * height * channels);

            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture: " << request.path << std::endl;
        }

        return result;
    }

private:
    std::thread worker;

    std::mutex mutex;
    std::condition_variable cvRequestMade;

    std::queue<TextureLoadRequest> requests;
    std::deque<TextureLoadResult> results;

    std::atomic<bool> stopFlag{false};
    std::atomic<bool> stopped{false};
};
