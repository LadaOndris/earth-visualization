

#include "ResourceFetcher.h"

std::queue<TextureLoadRequest> loadingTexturesQueue;
std::deque<TextureLoadResult> resultsQueue;
std::mutex loadingMutex;
std::mutex resultsMutex;
std::condition_variable cv;
std::atomic<bool> stopThread(false);
