//
// Created by lada on 10/31/23.
//

#include "ResourceFetcher.h"

std::queue<TextureLoadRequest> loadingTexturesQueue;
std::deque<TextureLoadResult> resultsQueue;
std::mutex loadingMutex;
std::mutex resultsMutex;
std::condition_variable cv;
