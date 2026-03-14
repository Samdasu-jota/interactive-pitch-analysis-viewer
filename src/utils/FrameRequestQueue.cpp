#include "FrameRequestQueue.h"

void FrameRequestQueue::push(int frameNumber) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(frameNumber);
    }
    cv_.notify_one();
}

int FrameRequestQueue::popLatest() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock, [this] {
        return !queue_.empty() || stopped_.load();
    });

    if (stopped_.load() && queue_.empty()) {
        return -1;
    }

    // Take the most recent request and discard all stale ones.
    int latest = queue_.back();
    queue_.clear();
    return latest;
}

void FrameRequestQueue::stop() {
    stopped_.store(true);
    cv_.notify_all();
}
