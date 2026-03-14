#include "PrefetchService.h"
#include <chrono>

PrefetchService::PrefetchService(RequestFrameFn requestFn, int lookahead)
    : requestFn_(std::move(requestFn))
    , lookahead_(lookahead) {
    running_.store(true);
    thread_ = std::thread(&PrefetchService::prefetchLoop, this);
}

PrefetchService::~PrefetchService() {
    stop();
}

void PrefetchService::setPosition(int currentFrame, int totalFrames) {
    currentFrame_.store(currentFrame);
    totalFrames_.store(totalFrames);
    dirty_.store(true);
    cv_.notify_one();
}

void PrefetchService::stop() {
    running_.store(false);
    cv_.notify_all();
    if (thread_.joinable()) thread_.join();
}

void PrefetchService::prefetchLoop() {
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return dirty_.load() || !running_.load(); });

        if (!running_.load()) break;
        dirty_.store(false);

        int start  = currentFrame_.load();
        int total  = totalFrames_.load();
        int end    = std::min(start + lookahead_, total - 1);

        lock.unlock();

        // Request frames ahead of the playhead.
        // The VideoDecodeService's FrameCache will absorb the requests
        // without visible latency because they arrive before paintEvent needs them.
        for (int frame = start + 1; frame <= end && running_.load(); ++frame) {
            requestFn_(frame);
            // Yield briefly to avoid saturating the decode thread queue
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}
